#include "JkBmsFrames.h"
#include <string.h>

namespace JkBmsCpp {

    Expected<JkBmsCellInfoResponse, JkBmsControllerError> parseCellsInfo(
        const JkBmsDataBuffer& buffer) {
        if (!checkFrameStart(buffer)) {
            return JkBmsControllerError::INVALID_MAGIC_BYTES;
        }
        if (!checkFrameChecksum(buffer)) {
            return JkBmsControllerError::CRC_MISMATCH;
        }
        if (buffer.size() != 300) {
            return JkBmsControllerError::INVALID_RESPONSE_TYPE;
        }
        const uint8_t* data = (const uint8_t*)buffer.data();
        if (buffer.data()[4] != static_cast<uint8_t>(JkBmsResponseType::CELL_INFO)) {
            return JkBmsControllerError::INVALID_RESPONSE_TYPE;
        }
        JkBmsCellInfoResponse response;
        uint8_t cellCount = JkBmsCellInfoResponse::CELL_COUNT;
        uint16_t pos = 6;

        // read cell voltages
        for (size_t i = 0; i < cellCount; i++) {
            response.cellVoltages_mV[i] = getUInt16LE(data, pos);
            pos += 2;
        }
        response.enabledCellMask = getUInt32LE(data, pos);
        pos += 4;
        response.averageCellVoltage_mV = getUInt16LE(data, pos);
        pos += 2;
        response.deltaCellVoltage_mV = getUInt16LE(data, pos);
        pos += 2;
        response.maxVoltageCellIndex = data[pos++];
        response.minVoltageCellIndex = data[pos++];

        for (size_t i = 0; i < cellCount; i++) {
            response.cellResistances_mOhm[i] = getUInt16LE(data, pos);
            pos += 2;
        }
        response.powerTubeTemperature = getUInt16LE(data, pos);
        pos += 2;
        response.wireResistanceWarningMask = getUInt32LE(data, pos);
        pos += 4;
        response.batteryVoltage_mV = getUInt32LE(data, pos);
        pos += 4;
        response.batteryPower_mW = getInt32LE(data, pos);
        pos += 4;
        response.chargeCurrent_mA = getInt32LE(data, pos);
        pos += 4;
        response.temperatureSensor1 = getInt16LE(data, pos);
        pos += 2;
        response.temperatureSensor2 = getInt16LE(data, pos);
        pos += 2;        
        response.errorsMask = getUInt16LE(data, pos);
        pos += 2;
        pos += 2; // skip unknown bytes
        response.balanceCurrent_mA = getInt16LE(data, pos);
        pos += 2;
        response.balanceAction = data[pos++];
        response.batteryPercentage = data[pos++];
        // not yet implemented fields
        response.remainingCapacity_mAh = 0;
        response.fullCapacity_mAh = 0;
        response.cycleCount = 0;
        response.totalCycleCapacity_mAh = 0;
        response.sohPercentage = 0;
        response.totalRunTime = 0;
        response.chargingState = 0;
        response.dischargingState = 0;
        response.emergencyTimeCountdown_s = 0;

        return response;
    }

    Expected<JkBmsDeviceInfoResponse, JkBmsControllerError> parseDeviceInfo(
        const JkBmsDataBuffer& buffer) {
        if (!checkFrameStart(buffer)) {
            return JkBmsControllerError::INVALID_MAGIC_BYTES;
        }
        if (!checkFrameChecksum(buffer)) {
            return JkBmsControllerError::CRC_MISMATCH;
        }
        if (buffer.size() != 300) {
            return JkBmsControllerError::INVALID_RESPONSE_TYPE;
        }
        const char* data = (const char*)buffer.data();
        if (data[4] != static_cast<char>(JkBmsResponseType::DEVICE_INFO)) {
            return JkBmsControllerError::INVALID_RESPONSE_TYPE;
        }
        JkBmsDeviceInfoResponse response;
        // Parsing logic to extract fields from buffer
        response.vendorId = JkBmsString(data + 6, strnlen(data + 6, 16));
        response.hwVersion = JkBmsString(data + 22, strnlen(data + 22, 8));
        response.swVersion = JkBmsString(data + 30, strnlen(data + 30, 8));
        response.uptimeSeconds = getUInt32LE(buffer.data(), 38);
        response.powerOnCounter = getUInt32LE(buffer.data(), 42);
        response.deviceName = JkBmsString(data + 46, strnlen(data + 46, 16));
        response.devicePasscode = JkBmsString(data + 62, strnlen(data + 62, 16));
        response.manufacturedate = JkBmsString(data + 78, strnlen(data + 78, 8));
        response.serialNumber = JkBmsString(data + 86, strnlen(data + 86, 11));
        response.passcode = JkBmsString(data + 97, strnlen(data + 97, 5));
        response.userData = JkBmsString(data + 102, strnlen(data + 102, 16));
        response.setupPasscode = JkBmsString(data + 118, strnlen(data + 118, 16));
        return response;
    }

    bool checkFrameStart(const JkBmsDataBuffer &buffer) {
        if (buffer.size() < 4) {
            return false;
        }
        const uint8_t* data = buffer.data();
        return data[0] == 0x55 && data[1] == 0xaa && data[2] == 0xeb && data[3] == 0x90;
    }

    bool checkFrameChecksum(const JkBmsDataBuffer &buffer) {
        if (buffer.size() < 5) {
            return false;
        }
        const uint8_t* data = buffer.data();
        uint8_t checksum = 0;
        for (size_t i = 0; i < buffer.size() - 1; i++) {
            checksum += data[i];
        }
        return checksum == data[buffer.size() - 1];
    }

    Expected<JkBmsResponseType, JkBmsControllerError> getResponseType(
        const JkBmsDataBuffer& buffer
    ) {
        if (!checkFrameStart(buffer)) {
            return JkBmsControllerError::INVALID_MAGIC_BYTES;
        }
        if (buffer.size() < 5) {
            return JkBmsControllerError::INVALID_RESPONSE_TYPE;
        }
        const uint8_t* data = buffer.data();
        JkBmsResponseType responseType = static_cast<JkBmsResponseType>(data[4]);
        switch (responseType) {
            case JkBmsResponseType::SETTINGS:
            case JkBmsResponseType::CELL_INFO:
            case JkBmsResponseType::DEVICE_INFO:
                return responseType;
            default:
                return JkBmsControllerError::INVALID_RESPONSE_TYPE;
        }
    }

    uint8_t calculateCRC(const JkBmsDataBuffer& buffer) {
        uint8_t crc = 0;
        for (uint16_t i = 0; i < buffer.size(); i++) {
            crc += buffer.data()[i];
        }
        return crc;
    }

    JkBmsControllerError prepareCommandBuffer(uint8_t reg, 
        uint32_t value,
        uint8_t length,
        const JkBmsDataBuffer& outBuffer) {
        if (outBuffer.size() < 20) {
            return JkBmsControllerError::BUFFER_TOO_SMALL;
        }
        constexpr size_t CRC_FRAME_SIZE = 19;
        uint8_t* frame = outBuffer.data();
        memset(frame, 0, outBuffer.size());
        frame[0] = 0xAA;     // start sequence
        frame[1] = 0x55;     // start sequence
        frame[2] = 0x90;     // start sequence
        frame[3] = 0xEB;     // start sequence
        frame[4] = reg;     // holding register
        frame[5] = length;   // size of the value in byte
        frame[6] = value >> 0;
        frame[7] = value >> 8;
        frame[8] = value >> 16;
        frame[9] = value >> 24;
        frame[19] = calculateCRC(JkBmsDataBuffer(frame, CRC_FRAME_SIZE));
        return JkBmsControllerError::SUCCESS;
    }
};