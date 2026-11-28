// Copyright 2026 Volodymyr Shkriabets (vshcryabets@gmail.com)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifndef JK_BMS_STRUCTURE_CELLS_COUNT
#define JK_BMS_STRUCTURE_CELLS_COUNT 32
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef char UuidStr[37];

typedef struct {
    uint8_t* ptr;
    uint16_t len;
} JkBmsDataBuffer;

typedef enum {
    CONTROLLER_ERROR_SUCCESS = 0,
    CONTROLLER_ERROR_NO_SOURCE = 1,
    CONTROLLER_ERROR_CRC_MISMATCH = 2,
    CONTROLLER_ERROR_INVALID_MAGIC_BYTES = 3,
    CONTROLLER_ERROR_INVALID_RESPONSE_TYPE = 4,
    CONTROLLER_ERROR_BUFFER_TOO_SMALL = 5,
} ControllerError;

typedef enum {
    SOURCE_ERROR_SUCCESS = 0,
    SOURCE_ERROR_FAILURE = 1,
    SOURCE_ERROR_NOT_CONNECTED = 2,
    SOURCE_ERROR_ALREADY_CONNECTED = 3,
    SOURCE_ERROR_NO_SUCH_SERVICE = 4,
    SOURCE_ERROR_NO_SUCH_CHARACTERISTIC = 5,
} SourceError;

typedef struct {
    uint16_t cellVoltages_mV[JK_BMS_STRUCTURE_CELLS_COUNT];
    uint32_t enabledCellMask;
    uint16_t averageCellVoltage_mV;
    uint16_t deltaCellVoltage_mV;
    uint8_t maxVoltageCellIndex;
    uint8_t minVoltageCellIndex;
    uint16_t cellResistances_mOhm[JK_BMS_STRUCTURE_CELLS_COUNT];
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
} CellInfoResponse;

typedef void (*CellInfoResponseCallback)(void* context, const CellInfoResponse* response);

typedef struct Source_C Source_C;

typedef SourceError (*Source_C_connect_fn)(Source_C* self);
typedef SourceError (*Source_C_disconnect_fn)(Source_C* self);
typedef SourceError (*Source_C_sendCommand_fn)(
    Source_C* self,
    const JkBmsDataBuffer command,
    const UuidStr service_uuid,
    const UuidStr char_uuid
);
typedef SourceError (*Source_C_subscribe_fn)(
    Source_C* self,
    const UuidStr service_uuid,
    const UuidStr char_uuid,
    void* context,
    void(*callback)(void* context, const JkBmsDataBuffer data)
);
typedef SourceError (*Source_C_unsubscribe_fn)(
    Source_C* self,
    const UuidStr service_uuid,
    const UuidStr char_uuid
);
typedef size_t (*Source_C_getMtu_fn)(Source_C* self);

typedef struct {
    SourceError (*connect)(Source_C* self);
    SourceError (*disconnect)(Source_C* self);
    SourceError (*sendCommand)(
        Source_C* self,
        const JkBmsDataBuffer command,
        const UuidStr service_uuid,
        const UuidStr char_uuid
    );
    SourceError (*subscribe)(
        Source_C* self,
        const UuidStr service_uuid,
        const UuidStr char_uuid,
        void* context,
        void(*callback)(void* context, const JkBmsDataBuffer data)
    );
    SourceError (*unsubscribe)(
        Source_C* self,
        const UuidStr service_uuid,
        const UuidStr char_uuid
    );
    size_t (*getMtu)(Source_C* self);
    void* vtable_reserved;
} Source_C_VTable;

struct Source_C {
    Source_C_VTable* vtable;
    void* data;
};

static inline SourceError Source_C_connect(Source_C* self) {
    return self->vtable->connect(self);
}

static inline SourceError Source_C_disconnect(Source_C* self) {
    return self->vtable->disconnect(self);
}

static inline SourceError Source_C_sendCommand(
    Source_C* self,
    const JkBmsDataBuffer command,
    const UuidStr service_uuid,
    const UuidStr char_uuid
) {
    return self->vtable->sendCommand(self, command, service_uuid, char_uuid);
}

static inline SourceError Source_C_subscribe(
    Source_C* self,
    const UuidStr service_uuid,
    const UuidStr char_uuid,
    void* context,
    void(*callback)(void* context, const JkBmsDataBuffer data)
) {
    return self->vtable->subscribe(self, service_uuid, char_uuid, context, callback);
}

static inline SourceError Source_C_unsubscribe(
    Source_C* self,
    const UuidStr service_uuid,
    const UuidStr char_uuid
) {
    return self->vtable->unsubscribe(self, service_uuid, char_uuid);
}

static inline size_t Source_C_getMtu(Source_C* self) {
    return self->vtable->getMtu(self);
}

static inline uint16_t getUInt16LE(const uint8_t* data, size_t pos) {
    return (uint16_t)data[pos] | ((uint16_t)data[pos + 1] << 8);
}

static inline int16_t getInt16LE(const uint8_t* data, size_t pos) {
    return (int16_t)(data[pos] | ((uint16_t)data[pos + 1] << 8));
}

static inline uint32_t getUInt32LE(const uint8_t* data, size_t pos) {
    return (uint32_t)data[pos] | 
           ((uint32_t)data[pos + 1] << 8) | 
           ((uint32_t)data[pos + 2] << 16) | 
           ((uint32_t)data[pos + 3] << 24);
}

static inline int32_t getInt32LE(const uint8_t* data, size_t pos) {
    return (int32_t)(data[pos] | 
            ((uint32_t)data[pos + 1] << 8) | 
            ((uint32_t)data[pos + 2] << 16) | 
            ((uint32_t)data[pos + 3] << 24));
}

bool checkFrameStart(const JkBmsDataBuffer buffer);
bool checkFrameChecksum(const JkBmsDataBuffer buffer);
uint8_t calculateCRC(const JkBmsDataBuffer buffer);
ControllerError prepareCommandBuffer(uint8_t reg, 
    uint32_t value,
    uint8_t length,
    const JkBmsDataBuffer outBuffer);

#ifdef __cplusplus
}
#endif
