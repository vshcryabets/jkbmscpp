#include "JkBmsFrames.h"
#include <string.h>

namespace JkBmsCpp {

    Expected<CellInfoResponse, ControllerError_C> 
    parseCellsInfo(
        const JkBmsDataBuffer_C buffer,
        uint8_t cellCount
    ) {
        if (!checkFrameStart(buffer)) {
            return CONTROLLER_ERROR_INVALID_MAGIC_BYTES;
        }
        if (!checkFrameChecksum(buffer)) {
            return CONTROLLER_ERROR_CRC_MISMATCH;
        }
        if (buffer.len != 300) {
            return CONTROLLER_ERROR_INVALID_RESPONSE_TYPE;
        }
        const uint8_t* data = (const uint8_t*)buffer.ptr;
        if (buffer.ptr[4] != static_cast<uint8_t>(ResponseType::CELL_INFO)) {
            return CONTROLLER_ERROR_INVALID_RESPONSE_TYPE;
        }
        CellInfoResponse response;
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
        response.remainingCapacity_mAh = getUInt32LE(data, pos);
        pos += 4;
        response.fullCapacity_mAh = getUInt32LE(data, pos);
        pos += 4;
        // not yet implemented fields
        response.cycleCount = 0;
        response.totalCycleCapacity_mAh = 0;
        response.sohPercentage = 0;
        response.totalRunTime = 0;
        response.chargingState = 0;
        response.dischargingState = 0;
        response.emergencyTimeCountdown_s = 0;

        return response;
    }

    Expected<DeviceInfoResponse, ControllerError_C> parseDeviceInfo(
        const JkBmsDataBuffer_C buffer) {
        if (!checkFrameStart(buffer)) {
            return CONTROLLER_ERROR_INVALID_MAGIC_BYTES;
        }
        if (!checkFrameChecksum(buffer)) {
            return CONTROLLER_ERROR_CRC_MISMATCH;
        }
        if (buffer.len != 300) {
            return CONTROLLER_ERROR_INVALID_RESPONSE_TYPE;
        }
        const char* data = (const char*)buffer.ptr;
        if (data[4] != static_cast<char>(ResponseType::DEVICE_INFO)) {
            return CONTROLLER_ERROR_INVALID_RESPONSE_TYPE;
        }
        DeviceInfoResponse response;
        // Parsing logic to extract fields from buffer
        response.vendorId = JkBmsString(data + 6, strnlen(data + 6, 16));
        response.hwVersion = JkBmsString(data + 22, strnlen(data + 22, 8));
        response.swVersion = JkBmsString(data + 30, strnlen(data + 30, 8));
        response.uptimeSeconds = getUInt32LE(buffer.ptr, 38);
        response.powerOnCounter = getUInt32LE(buffer.ptr, 42);
        response.deviceName = JkBmsString(data + 46, strnlen(data + 46, 16));
        response.devicePasscode = JkBmsString(data + 62, strnlen(data + 62, 16));
        response.manufacturedate = JkBmsString(data + 78, strnlen(data + 78, 8));
        response.serialNumber = JkBmsString(data + 86, strnlen(data + 86, 11));
        response.passcode = JkBmsString(data + 97, strnlen(data + 97, 5));
        response.userData = JkBmsString(data + 102, strnlen(data + 102, 16));
        response.setupPasscode = JkBmsString(data + 118, strnlen(data + 118, 16));
        return response;
    }

    Expected<ResponseType, ControllerError_C> getResponseType(
        const JkBmsDataBuffer_C buffer
    ) {
        if (!checkFrameStart(buffer)) {
            return CONTROLLER_ERROR_INVALID_MAGIC_BYTES;
        }
        if (buffer.len < 5) {
            return CONTROLLER_ERROR_INVALID_RESPONSE_TYPE;
        }
        const uint8_t* data = buffer.ptr;
        ResponseType responseType = static_cast<ResponseType>(data[4]);
        switch (responseType) {
            case ResponseType::SETTINGS:
            case ResponseType::CELL_INFO:
            case ResponseType::DEVICE_INFO:
                return responseType;
            default:
                return CONTROLLER_ERROR_INVALID_RESPONSE_TYPE;
        }
    }

};