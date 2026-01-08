#include "JkBmsFrames.h"

namespace JkBmsCpp {

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
        char* data = (char*)buffer.data();
        if (data[4] != static_cast<char>(JkBmsResponseType::DEVICE_INFO)) {
            return JkBmsControllerError::INVALID_RESPONSE_TYPE;
        }
        JkBmsDeviceInfoResponse response;
        // Parsing logic to extract fields from buffer
        // This is a placeholder implementation
        response.vendorId = JkBmsString(data + 6, strnlen(data + 6, 16));
        response.hwVersion = JkBmsString(data + 22, strnlen(data + 22, 8));
        response.swVersion = JkBmsString(data + 30, strnlen(data + 30, 8));
        response.uptimeSeconds = (uint8_t)data[38] |
            ((uint8_t)data[39] << 8) | 
            ((uint8_t)data[40] << 16) | 
            ((uint8_t)data[41] << 24);
        response.powerOnCounter = data[42] | 
            (data[43] << 8) | 
            (data[44] << 16) | 
            (data[45] << 24);
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
};