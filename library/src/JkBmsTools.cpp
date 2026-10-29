#include "JkBmsTools.h"

#include <cstdio>
#include <algorithm>

namespace JkBmsCpp {
    void dumpDataToLog(const uint8_t* data, const uint16_t size, Log& log) {
        constexpr size_t bytes_per_line = 16;
        size_t lines = (size + bytes_per_line - 1) / bytes_per_line;
        for (size_t line = 0; line < lines; line++) {
            size_t line_start = line * bytes_per_line;
            size_t line_end = std::min(line_start + bytes_per_line, (size_t)size);
            
            // Build the hex part
            char hex_part[256] = {0};
            char* hex_ptr = hex_part;
            hex_ptr += sprintf(hex_ptr, "%04lx: ", line_start);
            for (size_t i = line_start; i < line_end; i++) {
                hex_ptr += sprintf(hex_ptr, "%02x ", (int)(unsigned char)data[i]);
            }
            hex_ptr += sprintf(hex_ptr, " ");
            
            // Build the ascii part
            for (size_t i = line_start; i < line_end; i++) {
                char c = (data[i] >= 32 && data[i] <= 126) ? (char)data[i] : '.';
                hex_ptr += sprintf(hex_ptr, "%c", c);
            }
            
            log.info("%s", hex_part);
        }
    }
}