#pragma once

#include "CppWrappers.h"

namespace JkBmsCpp {
    enum class JkBmsSourceError: uint8_t {
        SUCCESS = 0,
    };

    enum class JkBmsControllerError: uint8_t {
        SUCCESS = 0,
        ERROR_NO_SOURCE = 1,
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
    JkBmsDeviceInfoResponse parseDeviceInfo(JkBmsDataBuffer);

};