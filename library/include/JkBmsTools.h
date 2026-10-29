#pragma once

#include <cstdint>
#include "CppWrappers.h"

namespace JkBmsCpp {
    void dumpDataToLog(const uint8_t* data, const uint16_t size, Log& log);
}