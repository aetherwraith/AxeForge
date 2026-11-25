#include "notify.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "esp_app_desc.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"

#include "core/json_util.h"
#include "core/settings.h"
#include "core/state.h"
#include "net/http_util.h"
#include "net/wifi.h"

static const char *TAG = "notify";

#define MSG_LEN 220
struct Msg {
    char text[MSG_LEN];
};
static QueueHandle_t s_q;

static bool send_json(const char *url, const char *body)
{
    int status = http::post(url, body, strlen(body), "application/json", 10000, true);
    if (status < 200 || status >= 300) {
        ESP_LOGW(TAG, "send failed (HTTP %d)", status);
        return false;
    }
    return true;
}

static void deliver(const char *text)
{
    const Settings &s = settings::get();
    char line[MSG_LEN + 48], esc[MSG_LEN * 2 + 64];
    snprintf(line, sizeof(line), "%s: %s", g_state.hostname[0] ? g_state.hostname : "AxeForge", text);
    json_escape(line, esc, sizeof(esc));
    if (s.discord_url[0]) {
        char body[sizeof(esc) + 32];
        snprintf(body, sizeof(body), "{\"content\":\"%s\"}", esc);
        send_json(s.discord_url, body);
    }
    if (s.tg_token[0] && s.tg_chat[0]) {
        char url[128], chat_esc[64], body[sizeof(esc) + 112];
        snprintf(url, sizeof(url), "https://api.telegram.org/bot%s/sendMessage", s.tg_token);
        json_escape(s.tg_chat, chat_esc, sizeof(chat_esc));
        snprintf(body, sizeof(body), "{\"chat_id\":\"%s\",\"text\":\"%s\"}", chat_esc, esc);
        send_json(url, body);
    }
}

static void task(void *)
{
    bool boot_sent = false;
    uint32_t pool_down_since = 0;
    bool pool_down_sent = false;
    Msg m;
    while (true) {
        if (xQueueReceive(s_q, &m, pdMS_TO_TICKS(5000)) == pdTRUE) {
            if (wifi::connected()) deliver(m.text);
            vTaskDelay(pdMS_TO_TICKS(2000));  // Discord allows ~5 messages per 2 s; stay well under
            continue;
        }
        // events that are a state rather than a moment are watched here
        if (!wifi::connected()) continue;
        if (!boot_sent && g_state.uptime_s > 20) {
            boot_sent = true;
            notify::post(NOTIFY_BOOT, "started AxeForge %s (last reset: %s)", esp_app_get_description()->version,
                         g_state.reset_reason[0] ? g_state.reset_reason : "power on");
        }
        bool down = !g_state.pool_connected && g_state.status == SysStatus::Mining;
        if (down) {
            if (!pool_down_since) pool_down_since = g_state.uptime_s;
            if (!pool_down_sent && g_state.uptime_s - pool_down_since >= 300) {
                pool_down_sent = true;
                notify::post(NOTIFY_POOL, "no pool has given work for 5 minutes");
            }
        } else {
            if (pool_down_sent) notify::post(NOTIFY_POOL, "mining again, a pool is back");
            pool_down_since = 0;
            pool_down_sent = false;
        }
        static uint32_t s_chip_warn_s[AF_NUM_ASICS] = {};
        static uint8_t s_chip_bad_cnt[AF_NUM_ASICS] = {};
        static uint32_t s_last_chip_chk = 0;
        if (g_state.status == SysStatus::Mining && g_state.uptime_s > 120 && g_state.uptime_s - s_last_chip_chk >= 15) {
            s_last_chip_chk = g_state.uptime_s;
            const Settings &cfg = settings::get();
            uint8_t nchips = g_state.chips_detected;
            if (nchips > AF_NUM_ASICS) nchips = AF_NUM_ASICS;
            for (uint8_t i = 0; i < nchips; i++) {
                float tot = g_state.chip_ghs[i] + g_state.chip_err_ghs[i];
                float ep = tot > 0 ? 100.0f * g_state.chip_err_ghs[i] / tot : 0;
                bool high_err = cfg.chip_err_warn_pct > 0 && ep > cfg.chip_err_warn_pct;
                bool lag_hr = false;
                float sib_avg = 0;
                if (nchips > 1) {
                    float sum = 0;
                    for (uint8_t j = 0; j < nchips; j++) if (j != i) sum += g_state.chip_ghs[j];
                    sib_avg = sum / (nchips - 1);
                    if (sib_avg > 10.0f && g_state.chip_ghs[i] < sib_avg * 0.85f) lag_hr = true;
                }
                if (high_err || lag_hr) {
                    if (s_chip_bad_cnt[i] < 255) s_chip_bad_cnt[i]++;
                    if (s_chip_bad_cnt[i] >= 4 && (!s_chip_warn_s[i] || g_state.uptime_s - s_chip_warn_s[i] >= 3600)) {
                        s_chip_warn_s[i] = g_state.uptime_s;
                        if (high_err)
                            notify::post(NOTIFY_MAINTENANCE, "chip %u error rate %.1f%% (limit %u%%)", i + 1, ep, cfg.chip_err_warn_pct);
                        else
                            notify::post(NOTIFY_MAINTENANCE, "chip %u hashrate %.0f GH/s (>15%% below siblings' %.0f GH/s)",
                                         i + 1, g_state.chip_ghs[i], sib_avg);
                    }
                } else {
                    s_chip_bad_cnt[i] = 0;
                }
            }
        }
    }
}

namespace notify {

void start()
{
    s_q = xQueueCreate(8, sizeof(Msg));
    // HTTPS needs a big stack; internal RAM is short, and this task never writes flash
    xTaskCreateWithCaps(task, "notify", 8192, NULL, 3, NULL, MALLOC_CAP_SPIRAM);
}

void post(uint8_t event, const char *fmt, ...)
{
    const Settings &s = settings::get();
    if (!s_q || !(s.notify_events & event)) return;
    if (!s.discord_url[0] && !(s.tg_token[0] && s.tg_chat[0])) return;
    Msg m;
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(m.text, sizeof(m.text), fmt, ap);
    va_end(ap);
    xQueueSend(s_q, &m, 0);  // a full queue drops the message rather than stall mining
}

bool test()
{
    const Settings &s = settings::get();
    if (!s_q || (!s.discord_url[0] && !(s.tg_token[0] && s.tg_chat[0]))) return false;
    Msg m;
    snprintf(m.text, sizeof(m.text), "test message. Notifications work.");
    xQueueSend(s_q, &m, 0);
    return true;
}

bool valid_tg_token(const char *t)
{
    // "123456789:AA..." : it goes into the URL path, so letters, digits, ':', '_' and '-' only
    size_t n = strlen(t);
    if (n < 20 || n >= sizeof(Settings::tg_token) || !strchr(t, ':')) return false;
    for (const char *p = t; *p; p++)
        if (!((*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || *p == ':' ||
              *p == '_' || *p == '-'))
            return false;
    return true;
}

bool valid_discord(const char *url)
{
    for (const char *p = url; *p; p++)
        if (*p <= ' ' || *p == '"' || *p == '\\') return false;
    return !strncmp(url, "https://discord.com/api/webhooks/", 33) ||
           !strncmp(url, "https://discordapp.com/api/webhooks/", 36) ||
           !strncmp(url, "https://ptb.discord.com/api/webhooks/", 37) ||
           !strncmp(url, "https://canary.discord.com/api/webhooks/", 40);
}

}
