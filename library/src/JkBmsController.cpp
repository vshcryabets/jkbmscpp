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
    responseBuffer.reserve(MAX_PACKET_SIZE);
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

uint8_t JkBmsController::calculateChecksum(const JkBmsDataBuffer& data) {
    uint8_t crc = 0;
    for (size_t i = 0; i < data.size(); i++) {
        crc += data.data()[i];
    }
    return crc;
}

CellInfoFuture JkBmsController::readCellsState() {
    pendingCellInfoRequest = std::make_unique<CellInfoPromise>();
    if (source == nullptr) {
        std::cerr << "Source is not started" << std::endl;
        pendingCellInfoRequest->set_value(
            Expected<JkBmsCellInfoResponse, JkBmsControllerError>(
                JkBmsControllerError::ERROR_NO_SOURCE));
        return pendingCellInfoRequest->get_future();
    }
    uint8_t frame[20];
    JkBmsDataBuffer command(frame, sizeof(frame));
    uint8_t length = 0;
    uint32_t value = 0;
    frame[0] = 0xAA;     // start sequence
    frame[1] = 0x55;     // start sequence
    frame[2] = 0x90;     // start sequence
    frame[3] = 0xEB;     // start sequence
    frame[4] = 0x96;  // holding register
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
    frame[19] = calculateChecksum(command);
    responseBuffer.clear();
    source->sendCommand(command, SERVICE_UUID, CHARACTERISTIC_UUID);
    return pendingCellInfoRequest->get_future();
}

DeviceInfoFuture JkBmsController::readDeviceState()
{
    pendingDeviceInfoRequest = std::make_unique<DeviceInfoPromise>();
    if (source == nullptr) {
        std::cerr << "Source is not started" << std::endl;
        pendingDeviceInfoRequest->set_value(
            Expected<JkBmsDeviceInfoResponse, JkBmsControllerError>(
                JkBmsControllerError::ERROR_NO_SOURCE));
        return pendingDeviceInfoRequest->get_future();
    }
    uint8_t frame[20];
    JkBmsDataBuffer command(frame, sizeof(frame));
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
    frame[19] = calculateChecksum(command);
    responseBuffer.clear();
    source->sendCommand(command, SERVICE_UUID, CHARACTERISTIC_UUID);
    return pendingDeviceInfoRequest->get_future();
}

void JkBmsController::notificationCallback(
            void* ctx,
            const JkBmsDataBuffer &data) {
    JkBmsController* controller = static_cast<JkBmsController*>(ctx);
    controller->handleResponse(data);
}

void JkBmsController::handleResponse(const JkBmsDataBuffer &data) {
    std::cout << "Received " << data.size() << " bytes:" << std::endl;
    dumpDataToLog(data.data(), data.size());
    if (checkFrameStart(data)) {
        // we received new packet, clear previous data
        std::cout << "New packet detected, clearing buffer" << std::endl;
        responseBuffer.clear();
    }
    responseBuffer.insert(responseBuffer.end(), data.data(), data.data() + data.size());
    if (responseBuffer.size() >= DEFAULT_PACKET_SIZE) {
        // we have full packet, process it
        JkBmsDataBuffer buffer(responseBuffer.data(), responseBuffer.size());
        if (!checkFrameChecksum(buffer)) {
            std::cerr << "Checksum mismatch" << std::endl;
            return;
        }
        auto responseType = getResponseType(buffer);
        if (!responseType.hasValue()) {
            std::cerr << "Error getting response type: "
                << static_cast<uint8_t>(responseType.error()) << std::endl;
            return;
        } else {
            std::cout << "Response type: " << (int)responseType.value() << std::endl;
            switch (responseType.value())
            {
            case JkBmsResponseType::DEVICE_INFO:
                {
                    if (pendingDeviceInfoRequest) {
                        auto deviceInfo = parseDeviceInfo(
                            JkBmsDataBuffer(responseBuffer.data(), responseBuffer.size()));
                        pendingDeviceInfoRequest->set_value(deviceInfo);
                        pendingDeviceInfoRequest.reset();
                    }
                }
                break;
            case JkBmsResponseType::CELL_INFO:
                {
                    if (pendingCellInfoRequest) {
                        auto cellInfo = parseCellsInfo(
                            JkBmsDataBuffer(responseBuffer.data(), responseBuffer.size()));
                        pendingCellInfoRequest->set_value(cellInfo);
                        pendingCellInfoRequest.reset();
                    }
                }
                break;
            default:
                break;
            }
            responseBuffer.clear();
        }
    }    
}

} // namespace JkBmsCpp