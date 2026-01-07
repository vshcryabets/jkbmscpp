#pragma once

#include "CppWrappers.h"

namespace JkBmsCpp {
    enum class JkBmsSourceError: uint8_t {
        SUCCESS = 0,
    };

    enum class JkBmsControllerError: uint8_t {
        SUCCESS = 0,
        ERROR_NO_SOURCE = 1,
        CRC_MISMATCH = 2,
        INVALID_MAGIC_BYTES = 3,
        INVALID_RESPONSE_TYPE = 4,
    };

    enum class JkBmsResponseType: uint8_t {
        CELL_INFO = 0x02,
        DEVICE_INFO = 0x03,
    };

    class JkBmsDeviceInfoResponse {
        public:
            JkBmsString vendorId;
            JkBmsString hwVersion;
            JkBmsString swVersion;
            uint32_t uptimeSeconds;
            uint32_t powerOnCounter;
            JkBmsString deviceName;
            JkBmsString devicePasscode;
            JkBmsString manufacturedate;
            JkBmsString serialNumber;
            JkBmsString passcode;
            JkBmsString userData;
            JkBmsString setupPasscode;
    };

    bool checkFrameStart(JkBmsDataBuffer);
    bool checkFrameChecksum(JkBmsDataBuffer);
    Expected<JkBmsDeviceInfoResponse, JkBmsControllerError> parseDeviceInfo(JkBmsDataBuffer);

};