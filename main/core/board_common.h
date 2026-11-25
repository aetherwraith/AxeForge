#pragma once
#include <stdint.h>
#include "core/board.h"

namespace board_common {

inline board::Error check_pmbus_faults(uint8_t sb, uint8_t si, uint8_t sv, uint8_t sn, uint8_t st)
{
    if (sb == 0xff && si == 0xff && sv == 0xff && sn == 0xff && st == 0xff) return board::Error::PsuFault;
    if (si != 0xff && (si & 0x80)) return board::Error::OverCurrent;
    if (sv != 0xff && (sv & 0x90)) return board::Error::VoutFault;
    if (st != 0xff && (st & 0x80)) return board::Error::VrOverTemp;
    if (sn != 0xff && (sn & 0x94)) return board::Error::PsuFault;
    return board::Error::None;
}

inline const char *error_text(board::Error e, const char *no_chips,
                              const char *no_reg = "Voltage regulator not found",
                              const char *psu_fault = "PSU / input voltage fault")
{
    switch (e) {
    case board::Error::None: return "OK";
    case board::Error::NoRegulator: return no_reg;
    case board::Error::NoTempSensor: return "Temperature sensor not responding";
    case board::Error::NoChips: return no_chips;
    case board::Error::ChipMissing: return "A chip in a stacked voltage domain didn't answer";
    case board::Error::PsuFault: return psu_fault;
    case board::Error::OverCurrent: return "Core over-current";
    case board::Error::VoutFault: return "Core voltage fault";
    case board::Error::VrOverTemp: return "Regulator over-temperature";
    }
    return "?";
}

} // namespace board_common
