#include <cstdint>
#include <iostream>
#include <iomanip>

#include "JkBmsController.h"
#include "JkBmsTools.h"

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

uint8_t JkBmsController::calculateChecksum(const uint8_t* data, const uint16_t size) {
    uint8_t crc = 0;
    for (size_t i = 0; i < size; i++) {
        crc += data[i];
    }
    return crc;
}

Expected<JkBmsDeviceInfoResponse, JkBmsControllerError> JkBmsController::readDeviceState()
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
    frame[19] = calculateChecksum(frame, 19);
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
    dumpDataToLog(data, size);
    
}
} // namespace JkBmsCpp