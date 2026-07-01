#include "screens/UiMappers.h"

#include <cstdio>
#include <cstdint>

void macAddressToString(const uint8_t macAddress[6], char* out, size_t outSize)
{
    snprintf(out, outSize, "%02X:%02X:%02X:%02X:%02X:%02X",
             macAddress[0], macAddress[1], macAddress[2],
             macAddress[3], macAddress[4], macAddress[5]);
}