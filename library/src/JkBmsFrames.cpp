#include "JkBmsFrames.h"

namespace JkBmsCpp {

    JkBmsDeviceInfoResponse parseDeviceInfo(JkBmsDataBuffer buffer) {
        JkBmsDeviceInfoResponse response;
        // Parsing logic to extract fields from buffer
        // This is a placeholder implementation
        response.vendorId = "Vendor123";
        response.hwVersion = "1.0";
        response.swVersion = "1.0.0";
        response.uptimeSeconds = 123456;
        response.powerOnCounter = 42;
        response.deviceName = "JK_BMS_Device";
        response.devicePasscode = "1234";
        response.manufacturedate = "2023-01-01";
        response.serialNumber = "SN123456789";
        response.passcode = "passcode";
        response.userData = "user_data";
        response.setupPasscode = "setup_passcode";
        return response;
    }

    bool checkFrameStart(JkBmsDataBuffer buffer) {
        if (buffer.size() < 4) {
            return false;
        }
        uint8_t* data = buffer.data();
        return data[0] == 0x55 && data[1] == 0xaa && data[2] == 0xeb && data[3] == 0x90;
    }

    bool checkFrameChecksum(JkBmsDataBuffer buffer) {
        if (buffer.size() < 5) {
            return false;
        }
        uint8_t* data = buffer.data();
        uint8_t checksum = 0;
        for (size_t i = 0; i < buffer.size() - 1; i++) {
            checksum += data[i];
        }
        return checksum == data[buffer.size() - 1];

    }


};