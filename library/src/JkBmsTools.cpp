#include "JkBmsTools.h"

#include <iostream>
#include <iomanip>

namespace JkBmsCpp {
    void dumpDataToLog(const uint8_t* data, const uint16_t size) {
        constexpr size_t bytes_per_line = 16;
        size_t lines = (size + bytes_per_line - 1) / bytes_per_line;
        for (size_t line = 0; line < lines; line++) {
            size_t line_start = line * bytes_per_line;
            size_t line_end = std::min(line_start + bytes_per_line, (size_t)size);
            std::cout << std::setw(4) << std::setfill('0') << std::hex << line_start << ": ";
            for (size_t i = line_start; i < line_end; i++) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') 
                        << (int)(unsigned char)data[i] << " ";
            }
            std::cout << std::dec << " ";
            for (size_t i = line_start; i < line_end; i++) {
                char c = (data[i] >= 32 && data[i] <= 126) ? (char)data[i] : '.';
                std::cout << c;
            }
            std::cout << std::endl;
        }
    }
}