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
        BUFER_TOO_SMALL = 5,
    };

    enum class JkBmsResponseType: uint8_t {
        SETTINGS = 0x01,
        CELL_INFO = 0x02,
        DEVICE_INFO = 0x03,
    };

    class JkBmsCellInfoResponse {
        public:
            constexpr static size_t CELL_COUNT = 32;
            uint16_t cellVoltages_mV[CELL_COUNT];
            uint32_t enabledCellMask;
            uint16_t averageCellVoltage_mV;
            uint16_t deltaCellVoltage_mV;
            uint8_t maxVoltageCellIndex;
            uint8_t minVoltageCellIndex;
            uint16_t cellResistances_mOhm[CELL_COUNT];
            int16_t powerTubeTemperature;
            uint32_t wireResistanceWarningMask;
            uint32_t batteryVoltage_mV;
            uint32_t batteryPower_mW;
            int32_t chargeCurrent_mA;
            int16_t temperatureSensor1;
            int16_t temperatureSensor2;
            uint16_t errorsMask;
            int16_t balanceCurrent_mA;
            uint8_t balanceAction;
            uint8_t batteryPercentage;
            uint32_t remainingCapacity_mAh;
            uint32_t fullCapacity_mAh;
            uint32_t cycleCount;
            uint32_t totalCycleCapacity_mAh;
            uint8_t sohPercentage;
            uint32_t totalRunTime;
            uint8_t chargingState;
            uint8_t dischargingState;
            uint16_t emergencyTimeCountdown_s;
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

    inline uint16_t getUInt16LE(const uint8_t* data, size_t pos) {
        return (uint16_t)data[pos] | ((uint16_t)data[pos + 1] << 8);
    }
    inline int16_t getInt16LE(const uint8_t* data, size_t pos) {
        return static_cast<int32_t>(data[pos] | ((uint16_t)data[pos + 1] << 8));
    }
    inline uint32_t getUInt32LE(const uint8_t* data, size_t pos) {
        return (uint32_t)data[pos] | 
            ((uint32_t)data[pos + 1] << 8) | 
            ((uint32_t)data[pos + 2] << 16) | 
            ((uint32_t)data[pos + 3] << 24);
    }
    inline int32_t getInt32LE(const uint8_t* data, size_t pos) {
        return static_cast<int32_t>(data[pos] | 
            ((uint32_t)data[pos + 1] << 8) | 
            ((uint32_t)data[pos + 2] << 16) | 
            ((uint32_t)data[pos + 3] << 24));
    }

    Expected<JkBmsResponseType, JkBmsControllerError> getResponseType(
        const JkBmsDataBuffer& buffer
    );
    bool checkFrameStart(const JkBmsDataBuffer& buffer);
    bool checkFrameChecksum(const JkBmsDataBuffer& buffer);
    Expected<JkBmsDeviceInfoResponse, JkBmsControllerError> parseDeviceInfo(
        const JkBmsDataBuffer& buffer
    );
    Expected<JkBmsCellInfoResponse, JkBmsControllerError> parseCellsInfo(
        const JkBmsDataBuffer& buffer
    );
    uint8_t calculateCRC(const JkBmsDataBuffer& buffer);
    JkBmsControllerError prepareCommandBuffer(uint8_t reg, 
        uint32_t value,
        uint8_t length,
        const JkBmsDataBuffer& outBuffer);

};