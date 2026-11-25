#pragma once
#include <math.h>
#include <stdint.h>

namespace pmbus_util {

inline float slinear11_to_float(uint16_t value)
{
    int32_t exponent = value >> 11;
    int32_t mantissa = value & 0x7ff;
    if (exponent & 0x10) exponent |= 0xffffffe0;
    if (mantissa & 0x400) mantissa |= 0xfffff800;
    return ldexpf((float) mantissa, exponent);
}

inline uint16_t float_to_slinear11(float x)
{
    if (x <= 0.0f) return 0;
    int32_t e = -16, m = 0;
    for (; e <= 15; e++) {
        m = (int32_t) roundf(ldexpf(x, -e));
        if (m >= 0 && m <= 1023) break;
    }
    if (e > 15) return 0;
    return (uint16_t) (((e & 0x1f) << 11) | (m & 0x7ff));
}

} // namespace pmbus_util
