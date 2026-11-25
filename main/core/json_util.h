#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "cJSON.h"

// cJSON parses recursively and allows 1000 levels of nesting, while the tasks
// that parse untrusted JSON (pools, Swarm peers, web handlers, weather/mempool
// replies) run on 6-12 KB stacks: a line of ~1000 '[' overflows them. Every
// JSON from outside goes through json_parse(), which refuses anything nested
// deeper than this before cJSON sees it. Nothing AxeForge reads needs more than 6.
#define JSON_MAX_DEPTH 16

// Deepest bracket nesting in `s`, ignoring brackets inside strings. Stops
// counting (and returns a value above `limit`) as soon as `limit` is passed.
inline int json_depth(const char *s, int limit)
{
    int depth = 0, max = 0;
    bool in_str = false;
    for (; *s; s++) {
        char c = *s;
        if (in_str) {
            if (c == '\\' && s[1]) s++;  // skip the escaped character
            else if (c == '"') in_str = false;
            continue;
        }
        if (c == '"') in_str = true;
        else if (c == '[' || c == '{') {
            if (++depth > max) max = depth;
            if (max > limit) return max;
        } else if (c == ']' || c == '}') {
            if (depth > 0) depth--;
        }
    }
    return max;
}

// cJSON_Parse for untrusted input: NULL (as for any parse error) when nested
// deeper than JSON_MAX_DEPTH.
inline cJSON *json_parse(const char *s)
{
    if (!s || json_depth(s, JSON_MAX_DEPTH) > JSON_MAX_DEPTH) return nullptr;
    return cJSON_Parse(s);
}

inline double json_num(cJSON *o, const char *k, double def = 0)
{
    cJSON *v = cJSON_GetObjectItem(o, k);
    return cJSON_IsNumber(v) ? v->valuedouble : def;
}

inline void json_str(cJSON *o, const char *k, char *dst, size_t len)
{
    if (!len) return;
    cJSON *v = cJSON_GetObjectItem(o, k);
    if (cJSON_IsString(v)) snprintf(dst, len, "%s", v->valuestring);
}

inline size_t json_escape(const char *src, char *dst, size_t cap)
{
    if (!cap) return 0;
    size_t o = 0;
    for (const char *p = src; *p && o + 6 < cap; p++) {
        uint8_t c = (uint8_t) *p;
        if (c == '"' || c == '\\') { dst[o++] = '\\'; dst[o++] = (char) c; }
        else if (c == '\n') { dst[o++] = '\\'; dst[o++] = 'n'; }
        else if (c == '\r') { dst[o++] = '\\'; dst[o++] = 'r'; }
        else if (c == '\t') { dst[o++] = '\\'; dst[o++] = 't'; }
        else if (c < 0x20) o += snprintf(dst + o, cap - o, "\\u%04x", c);
        else dst[o++] = (char) c;
    }
    dst[o] = 0;
    return o;
}
