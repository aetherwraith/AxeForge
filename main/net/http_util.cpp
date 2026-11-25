#include "http_util.h"

#include <stdlib.h>
#include <string.h>
#include "esp_crt_bundle.h"
#include "esp_heap_caps.h"
#include "esp_http_client.h"
#include "esp_log.h"

static const char *TAG = "http";

namespace http {

char *get(const char *url, size_t max_len, int timeout_ms, bool quiet)
{
    esp_http_client_config_t cfg = {};
    cfg.url = url;
    cfg.crt_bundle_attach = esp_crt_bundle_attach;
    cfg.timeout_ms = timeout_ms;
    cfg.buffer_size = 2048;
    esp_http_client_handle_t c = esp_http_client_init(&cfg);
    if (!c) {  // the URL did not parse or has no host
        if (!quiet) ESP_LOGW(TAG, "invalid URL: %s", url);
        return NULL;
    }
    esp_http_client_set_header(c, "Accept-Encoding", "identity");
    char *buf = NULL;
    if (esp_http_client_open(c, 0) == ESP_OK) {
        esp_http_client_fetch_headers(c);
        int status = esp_http_client_get_status_code(c);
        if (status == 200) {
            int64_t cl = esp_http_client_get_content_length(c);
            size_t alloc_len = (cl > 0 && (size_t) cl + 1 < max_len) ? (size_t) cl + 1 : max_len;
            buf = (char *) heap_caps_malloc(alloc_len, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
            if (!buf) buf = (char *) malloc(alloc_len);
            size_t total = 0;
            int n;
            while (buf && total < alloc_len - 1 && (n = esp_http_client_read(c, buf + total, alloc_len - 1 - total)) > 0)
                total += n;
            if (buf) buf[total] = 0;
        } else {
            // never the query: it may carry a token
            if (!quiet) ESP_LOGW(TAG, "HTTP %d for %.*s", status, (int) strcspn(url, "?"), url);
        }
    } else {
        if (!quiet) ESP_LOGW(TAG, "request failed: %.*s", (int) strcspn(url, "?"), url);
    }
    esp_http_client_cleanup(c);
    return buf;
}

int post(const char *url, const char *body, size_t len, const char *ctype, int timeout_ms, bool quiet)
{
    esp_http_client_config_t cfg = {};
    cfg.url = url;
    cfg.crt_bundle_attach = esp_crt_bundle_attach;
    cfg.timeout_ms = timeout_ms;
    cfg.buffer_size = 2048;
    cfg.method = HTTP_METHOD_POST;
    esp_http_client_handle_t c = esp_http_client_init(&cfg);
    if (!c) {
        if (!quiet) ESP_LOGW(TAG, "invalid URL: %s", url);
        return -1;
    }
    if (ctype) esp_http_client_set_header(c, "Content-Type", ctype);
    esp_http_client_set_header(c, "X-AxeForge", "1");
    int status = -1;
    if (esp_http_client_open(c, (int) len) == ESP_OK) {
        size_t off = 0;
        bool ok = true;
        while (off < len) {
            size_t chunk = len - off;
            if (chunk > 4096) chunk = 4096;
            int w = esp_http_client_write(c, body + off, (int) chunk);
            if (w <= 0) {
                ok = false;
                break;
            }
            off += (size_t) w;
        }
        if (ok) {
            esp_http_client_fetch_headers(c);
            status = esp_http_client_get_status_code(c);
        }
    } else if (!quiet) {
        ESP_LOGW(TAG, "POST failed: %.*s", (int) strcspn(url, "?"), url);
    }
    esp_http_client_cleanup(c);
    return status;
}

} // namespace http
