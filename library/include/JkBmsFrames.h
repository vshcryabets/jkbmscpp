#pragma once

#include <algorithm>
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

    /// @brief C++ wrapper for CellInfoResponse with equality operators
    /// 
    /// This class wraps the C struct CellInfoResponse and provides operator== and operator!=
    /// for use with reactive streams like StateFlow that require equality comparison.
    class CellInfo {
    public:
        CellInfoResponse data;

        /// @brief Default constructor
        CellInfo() { 
            data.enabledCellMask = 0;
            data.averageCellVoltage_mV = 0; 
            data.deltaCellVoltage_mV = 0;            
            data.maxVoltageCellIndex = 0;
            data.minVoltageCellIndex = 0;
            data.powerTubeTemperature = 0;
            data.wireResistanceWarningMask = 0;
            data.batteryVoltage_mV = 0;
            data.batteryPower_mW = 0;
            data.chargeCurrent_mA = 0;
            data.temperatureSensor1 = 0;
            data.temperatureSensor2 = 0;
            data.errorsMask = 0;
            data.balanceCurrent_mA = 0;
            data.balanceAction = 0;
            data.batteryPercentage = 0;
            data.remainingCapacity_mAh = 0;
            data.fullCapacity_mAh = 0;
            data.cycleCount = 0;
            data.totalCycleCapacity_mAh = 0;
            data.sohPercentage = 0;
            data.totalRunTime = 0;
            data.chargingState = 0;
            data.dischargingState = 0;
            data.emergencyTimeCountdown_s = 0;
            std::fill(std::begin(data.cellVoltages_mV), std::end(data.cellVoltages_mV), 0);
            std::fill(std::begin(data.cellResistances_mOhm), std::end(data.cellResistances_mOhm), 0);
        }

        /// @brief Construct from C struct
        /// @param c_response The C struct to copy from
        explicit CellInfo(const CellInfoResponse& c_response) {
            this->data = c_response;
        }

        /// @brief Assignment operator from C struct
        CellInfo& operator=(const CellInfoResponse& c_response) {
            this->data = c_response;
            return *this;
        }

        /// @brief Equality operator
        bool operator==(const CellInfo& other) const {
            return std::equal(std::begin(data.cellVoltages_mV), std::end(data.cellVoltages_mV),
                            std::begin(other.data.cellVoltages_mV)) &&
                   data.enabledCellMask == other.data.enabledCellMask &&
                   data.averageCellVoltage_mV == other.data.averageCellVoltage_mV &&
                   data.deltaCellVoltage_mV == other.data.deltaCellVoltage_mV &&
                   data.maxVoltageCellIndex == other.data.maxVoltageCellIndex &&
                   data.minVoltageCellIndex == other.data.minVoltageCellIndex &&
                   std::equal(std::begin(data.cellResistances_mOhm), std::end(data.cellResistances_mOhm),
                            std::begin(other.data.cellResistances_mOhm)) &&
                   data.powerTubeTemperature == other.data.powerTubeTemperature &&
                   data.wireResistanceWarningMask == other.data.wireResistanceWarningMask &&
                   data.batteryVoltage_mV == other.data.batteryVoltage_mV &&
                   data.batteryPower_mW == other.data.batteryPower_mW &&
                   data.chargeCurrent_mA == other.data.chargeCurrent_mA &&
                   data.temperatureSensor1 == other.data.temperatureSensor1 &&
                   data.temperatureSensor2 == other.data.temperatureSensor2 &&
                   data.errorsMask == other.data.errorsMask &&
                   data.balanceCurrent_mA == other.data.balanceCurrent_mA &&
                   data.balanceAction == other.data.balanceAction &&
                   data.batteryPercentage == other.data.batteryPercentage &&
                   data.remainingCapacity_mAh == other.data.remainingCapacity_mAh &&
                   data.fullCapacity_mAh == other.data.fullCapacity_mAh &&
                   data.cycleCount == other.data.cycleCount &&
                   data.totalCycleCapacity_mAh == other.data.totalCycleCapacity_mAh &&
                   data.sohPercentage == other.data.sohPercentage &&
                   data.totalRunTime == other.data.totalRunTime &&
                   data.chargingState == other.data.chargingState &&
                   data.dischargingState == other.data.dischargingState &&
                   data.emergencyTimeCountdown_s == other.data.emergencyTimeCountdown_s;
        }

        /// @brief Inequality operator
        bool operator!=(const CellInfo& other) const {
            return !(*this == other);
        }
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