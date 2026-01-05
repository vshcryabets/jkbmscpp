#include <cstdint>
#include <iostream>
#include <iomanip>

#include "JkBmsController.h"

namespace JkBmsCpp
{

const JkBmsString JkBmsController::SERVICE_UUID = "0000ffe0-0000-1000-8000-00805f9b34fb";
const JkBmsString JkBmsController::CHARACTERISTIC_UUID = "0000ffe1-0000-1000-8000-00805f9b34fb";


JkBmsController::JkBmsController()
{
}

JkBmsController::~JkBmsController()
{
}

void JkBmsController::start(JkBmsSource* source)
{
    this->source = source;
    this->source->connect();
    this->source->subscribe(
        SERVICE_UUID,
        CHARACTERISTIC_UUID,
        (void*)this,
        this->notificationCallback);
}

void JkBmsController::end()
{
    if (this->source)
    {
        this->source->unsubscribe(
            SERVICE_UUID,
            CHARACTERISTIC_UUID);
        this->source->disconnect();
        this->source = nullptr;
    }
}

JkBmsControllerError JkBmsController::readDeviceState()
{
    if (source == nullptr) {
        std::cerr << "Source is not started" << std::endl;
        return JkBmsControllerError::ERROR_NO_SOURCE;
    }
    uint8_t frame[20];
    uint8_t length = 0;
    uint32_t value = 0;
    frame[0] = 0xAA;     // start sequence
    frame[1] = 0x55;     // start sequence
    frame[2] = 0x90;     // start sequence
    frame[3] = 0xEB;     // start sequence
    frame[4] = 0x97;  // holding register
    frame[5] = length;   // size of the value in byte
    frame[6] = value >> 0;
    frame[7] = value >> 8;
    frame[8] = value >> 16;
    frame[9] = value >> 24;
    frame[10] = 0x00;
    frame[11] = 0x00;
    frame[12] = 0x00;
    frame[13] = 0x00;
    frame[14] = 0x00;
    frame[15] = 0x00;
    frame[16] = 0x00;
    frame[17] = 0x00;
    frame[18] = 0x00;
    uint8_t crc = 0;
    for (size_t i = 0; i < sizeof(frame) - 1; i++) {
        crc += frame[i];
    }
    frame[19] = crc;
    source->sendCommand(frame, sizeof(frame), SERVICE_UUID, CHARACTERISTIC_UUID);
    return JkBmsControllerError::SUCCESS;
}

void JkBmsController::notificationCallback(
            void* ctx,
            const uint8_t* data,
            const uint16_t size) {
    JkBmsController* controller = static_cast<JkBmsController*>(ctx);
    controller->handleResponse(data, size);
}

void JkBmsController::handleResponse(const uint8_t* data, const uint16_t size) {
    std::cout << "Received " << size << " bytes:" << std::endl;
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
} // namespace JkBmsCpp