#include "mqtt_ha.h"

#include <stdio.h>
#include <string.h>
#include "esp_app_desc.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt_client.h"

#include "core/board_config.h"
#include "core/settings.h"
#include "core/state.h"
#include "net/wifi.h"

static const char *TAG = "mqtt";

static esp_mqtt_client_handle_t s_client = nullptr;
static volatile bool s_connected = false;
static volatile bool s_paused = false;
static volatile bool s_reconfig = false;

static void publish_discovery()
{
    if (!s_client || !s_connected) return;
    const char *h = g_state.hostname[0] ? g_state.hostname : "axeforge";
    char state_t[64], cmd_t[64], topic[128], payload[512];
    snprintf(state_t, sizeof(state_t), "axeforge/%s/state", h);
    snprintf(cmd_t, sizeof(cmd_t), "axeforge/%s/cmd/mining", h);
    const char *ver = esp_app_get_description()->version;

    struct SensorDef {
        const char *id, *name, *unit, *dclass, *val_tpl;
    };
    static const SensorDef SENSORS[] = {
        {"hashrate", "Hashrate", "GH/s", nullptr, "{{ value_json.hashrate }}"},
        {"power", "Power", "W", "power", "{{ value_json.power }}"},
        {"chip_temp", "ASIC Temperature", "°C", "temperature", "{{ value_json.chipTemp }}"},
        {"vr_temp", "VR Temperature", "°C", "temperature", "{{ value_json.vrTemp }}"},
        {"freq", "Frequency", "MHz", nullptr, "{{ value_json.freq }}"},
        {"accepted", "Accepted Shares", nullptr, nullptr, "{{ value_json.accepted }}"},
        {"best_diff", "Best Difficulty", nullptr, nullptr, "{{ value_json.bestDiff }}"},
    };
    for (const auto &s : SENSORS) {
        snprintf(topic, sizeof(topic), "homeassistant/sensor/%s_%s/config", h, s.id);
        int n = snprintf(payload, sizeof(payload),
                         "{\"name\":\"%s\",\"uniq_id\":\"%s_%s\",\"stat_t\":\"%s\",\"val_tpl\":\"%s\"",
                         s.name, h, s.id, state_t, s.val_tpl);
        if (s.unit && n < (int) sizeof(payload)) n += snprintf(payload + n, sizeof(payload) - n, ",\"unit_of_meas\":\"%s\"", s.unit);
        if (s.dclass && n < (int) sizeof(payload)) n += snprintf(payload + n, sizeof(payload) - n, ",\"dev_cla\":\"%s\"", s.dclass);
        if (n < (int) sizeof(payload)) {
            snprintf(payload + n, sizeof(payload) - n,
                     ",\"dev\":{\"ids\":[\"%s\"],\"name\":\"%s\",\"mf\":\"AxeForge\",\"mdl\":\"%s\",\"sw\":\"%s\"}}",
                     h, h, AF_BOARD_NAME, ver);
        }
        esp_mqtt_client_publish(s_client, topic, payload, 0, 1, 1);
    }

    snprintf(topic, sizeof(topic), "homeassistant/switch/%s_mining/config", h);
    snprintf(payload, sizeof(payload),
             "{\"name\":\"Mining\",\"uniq_id\":\"%s_mining\",\"stat_t\":\"%s\",\"cmd_t\":\"%s\","
             "\"val_tpl\":\"{{ value_json.mining }}\",\"pl_on\":\"ON\",\"pl_off\":\"OFF\","
             "\"dev\":{\"ids\":[\"%s\"],\"name\":\"%s\",\"mf\":\"AxeForge\",\"mdl\":\"%s\",\"sw\":\"%s\"}}",
             h, state_t, cmd_t, h, h, AF_BOARD_NAME, ver);
    esp_mqtt_client_publish(s_client, topic, payload, 0, 1, 1);
}

static void publish_state()
{
    if (!s_client || !s_connected) return;
    const MinerState &s = g_state;
    const char *h = s.hostname[0] ? s.hostname : "axeforge";
    char topic[64], payload[256];
    snprintf(topic, sizeof(topic), "axeforge/%s/state", h);
    snprintf(payload, sizeof(payload),
             "{\"hashrate\":%.1f,\"power\":%.1f,\"chipTemp\":%.1f,\"vrTemp\":%.1f,"
             "\"freq\":%u,\"accepted\":%lu,\"bestDiff\":%.0f,\"mining\":\"%s\"}",
             s.hashrate_ghs, s.pin, s.chip_temp_max, s.vr_temp, (unsigned) s.freq_mhz,
             (unsigned long) s.shares_accepted, s.best_diff, s_paused ? "OFF" : "ON");
    esp_mqtt_client_publish(s_client, topic, payload, 0, 0, 0);
}

static void on_event(void *, esp_event_base_t, int32_t event_id, void *event_data)
{
    auto *ev = (esp_mqtt_event_handle_t) event_data;
    switch ((esp_mqtt_event_id_t) event_id) {
    case MQTT_EVENT_CONNECTED: {
        s_connected = true;
        ESP_LOGI(TAG, "connected");
        const char *h = g_state.hostname[0] ? g_state.hostname : "axeforge";
        char cmd_t[64];
        snprintf(cmd_t, sizeof(cmd_t), "axeforge/%s/cmd/mining", h);
        esp_mqtt_client_subscribe(s_client, cmd_t, 0);
        publish_discovery();
        publish_state();
        break;
    }
    case MQTT_EVENT_DISCONNECTED:
        s_connected = false;
        break;
    case MQTT_EVENT_DATA:
        if (ev->data && ev->data_len >= 2) {
            if (ev->data_len == 2 && strncasecmp(ev->data, "ON", 2) == 0) s_paused = false;
            else if (ev->data_len == 3 && strncasecmp(ev->data, "OFF", 3) == 0) s_paused = true;
            publish_state();
        }
        break;
    default:
        break;
    }
}

static void stop_client()
{
    if (!s_client) return;
    esp_mqtt_client_stop(s_client);
    esp_mqtt_client_destroy(s_client);
    s_client = nullptr;
    s_connected = false;
}

static void start_client()
{
    const Settings &cfg = settings::get();
    if (!cfg.mqtt_on || !cfg.mqtt_url[0]) return;
    esp_mqtt_client_config_t mcfg = {};
    mcfg.broker.address.uri = cfg.mqtt_url;
    if (cfg.mqtt_user[0]) mcfg.credentials.username = cfg.mqtt_user;
    if (cfg.mqtt_pass[0]) mcfg.credentials.authentication.password = cfg.mqtt_pass;
    s_client = esp_mqtt_client_init(&mcfg);
    if (!s_client) return;
    esp_mqtt_client_register_event(s_client, MQTT_EVENT_ANY, on_event, nullptr);
    esp_mqtt_client_start(s_client);
}

static void task(void *)
{
    int64_t last_pub_s = 0;
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        if (s_reconfig) {
            s_reconfig = false;
            stop_client();
            if (!settings::get().mqtt_on) s_paused = false;
        }
        if (!wifi::connected() || !settings::get().mqtt_on || !settings::get().mqtt_url[0]) {
            if (s_client) stop_client();
            continue;
        }
        if (!s_client) start_client();
        int64_t now_s = esp_timer_get_time() / 1000000;
        if (s_connected && now_s - last_pub_s >= 15) {
            last_pub_s = now_s;
            publish_state();
        }
    }
}

namespace mqtt_ha {

void start() { xTaskCreateWithCaps(task, "mqtt", 4096, NULL, 2, NULL, MALLOC_CAP_SPIRAM); }
void reconfigure() { s_reconfig = true; }
bool want_pause() { return s_paused; }

} // namespace mqtt_ha
