#pragma once

#include "CppWrappers.h"
#include "CFunctions.h"

namespace JkBmsCpp {
    enum class ResponseType: uint8_t {
        SETTINGS = 0x01,
        CELL_INFO = 0x02,
        DEVICE_INFO = 0x03,
    };

    class DeviceInfoResponse {
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

    Expected<ResponseType, ControllerError> getResponseType(
        const JkBmsDataBuffer buffer
    );
    Expected<DeviceInfoResponse, ControllerError> parseDeviceInfo(
        const JkBmsDataBuffer buffer
    );
    Expected<CellInfoResponse, ControllerError> 
    parseCellsInfo(
        const JkBmsDataBuffer buffer,
        uint8_t cellCount
    );

};