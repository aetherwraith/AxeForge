#pragma once
#include <stdio.h>
#include "lvgl.h"
#include "ui/theme.h"

namespace ui_fmt {

inline void fmt_hash(float ghs, char *val, size_t vl, const char **unit)
{
    if (ghs >= 1000.0f) {
        snprintf(val, vl, "%.2f", ghs / 1000.0f);
        *unit = "TH/s";
    } else {
        snprintf(val, vl, "%.0f", ghs);
        *unit = "GH/s";
    }
}

inline void fmt_diff(double d, char *out, size_t len, int prec = 2)
{
    static const char *const SUF[] = {"", "k", "M", "G", "T", "P", "E"};
    int i = 0;
    while (d >= 1000.0 && i < 6) {
        d /= 1000.0;
        i++;
    }
    if (!i) snprintf(out, len, "%.0f", d);
    else snprintf(out, len, "%.*f%s", prec, d, SUF[i]);
}

inline lv_color_t temp_color(float t, float warn, float hot, lv_color_t ok_col, lv_color_t warn_col, lv_color_t hot_col)
{
    if (t >= hot) return hot_col;
    if (t >= warn) return warn_col;
    return ok_col;
}

inline lv_color_t fee_color(float f)
{
    if (theme::ember()) return lv_color_hex(theme::mempool_fee_rgb(f));
    if (f < 2) return lv_color_hex(0x5b3fd9);
    if (f < 5) return lv_color_hex(0x3d7bff);
    if (f < 10) return lv_color_hex(0x1fbfa0);
    if (f < 20) return lv_color_hex(0x8fd12b);
    if (f < 50) return lv_color_hex(0xffb21c);
    return lv_color_hex(0xff3b5c);
}

inline void hash_markup(const char *h, int per_line, char *out, size_t n)
{
    int o = 0, z = 0;
    while (z < 64 && h[z] == '0') z++;
    bool col = false;
    for (int i = 0; i < 64 && o < (int) n - 12; i++) {
        if (i && i % per_line == 0) {
            if (col) out[o++] = '#';
            out[o++] = '\n';
            col = false;
        }
        if (i < z && !col) {
            o += snprintf(out + o, n - o, "#00e5ff ");
            col = true;
        }
        if (i == z && col) {
            out[o++] = '#';
            col = false;
        }
        out[o++] = h[i];
    }
    if (col) out[o++] = '#';
    out[o] = 0;
}

} // namespace ui_fmt
