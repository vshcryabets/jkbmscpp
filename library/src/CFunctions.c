#include "CFunctions.h"
#include <string.h>

bool checkFrameStart(const JkBmsDataBuffer_C buffer) {
    if (buffer.len < 4) {
        return false;
    }
    return buffer.ptr[0] == 0x55 && buffer.ptr[1] == 0xaa && 
           buffer.ptr[2] == 0xeb && buffer.ptr[3] == 0x90;
}

bool checkFrameChecksum(const JkBmsDataBuffer_C buffer) {
    if (buffer.len < 5) {
        return false;
    }
    uint8_t checksum = 0;
    for (uint16_t i = 0; i < buffer.len - 1; i++) {
        checksum += buffer.ptr[i];
    }
    return checksum == buffer.ptr[buffer.len - 1];
}

uint8_t calculateCRC(const JkBmsDataBuffer_C buffer) {
    uint8_t crc = 0;
    for (uint16_t i = 0; i < buffer.len; i++) {
        crc += buffer.ptr[i];
    }
    return crc;
}

ControllerError_C prepareCommandBuffer(uint8_t reg, 
    uint32_t value,
    uint8_t length,
    const JkBmsDataBuffer_C outBuffer) {
    if (outBuffer.len < 20) {
        return CONTROLLER_ERROR_BUFFER_TOO_SMALL;
    }
    const size_t CRC_FRAME_SIZE = 19;
    uint8_t* frame = outBuffer.ptr;
    memset(frame, 0, outBuffer.len);
    frame[0] = 0xAA;     // start sequence
    frame[1] = 0x55;     // start sequence
    frame[2] = 0x90;     // start sequence
    frame[3] = 0xEB;     // start sequence
    frame[4] = reg;      // holding register
    frame[5] = length;   // size of the value in byte
    frame[6] = value >> 0;
    frame[7] = value >> 8;
    frame[8] = value >> 16;
    frame[9] = value >> 24;
    JkBmsDataBuffer_C crcBuffer = {frame, CRC_FRAME_SIZE};
    frame[19] = calculateCRC(crcBuffer);
    return CONTROLLER_ERROR_SUCCESS;
}
