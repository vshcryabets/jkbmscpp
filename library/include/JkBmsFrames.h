#pragma once

#include "CppWrappers.h"
#include "CFunctions.h"

namespace JkBmsCpp {
    enum class ResponseType: uint8_t {
        SETTINGS = 0x01,
        CELL_INFO = 0x02,
        DEVICE_INFO = 0x03,
    };

    class CellInfoResponse {
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

    Expected<ResponseType, ControllerError_C> getResponseType(
        const JkBmsDataBuffer_C buffer
    );
    Expected<DeviceInfoResponse, ControllerError_C> parseDeviceInfo(
        const JkBmsDataBuffer_C buffer
    );
    Expected<CellInfoResponse, ControllerError_C> 
    parseCellsInfo(
        const JkBmsDataBuffer_C buffer,
        uint8_t cellCount
    );

};