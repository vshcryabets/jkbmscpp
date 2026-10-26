#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef char UuidStr[37];

typedef struct {
    uint8_t* ptr;
    uint16_t len;
} JkBmsDataBuffer_C;

typedef enum {
    CONTROLLER_ERROR_SUCCESS = 0,
    CONTROLLER_ERROR_NO_SOURCE = 1,
    CONTROLLER_ERROR_CRC_MISMATCH = 2,
    CONTROLLER_ERROR_INVALID_MAGIC_BYTES = 3,
    CONTROLLER_ERROR_INVALID_RESPONSE_TYPE = 4,
    CONTROLLER_ERROR_BUFFER_TOO_SMALL = 5,
} ControllerError_C;

typedef enum {
    SOURCE_ERROR_SUCCESS = 0,
    SOURCE_ERROR_FAILURE = 1,
    SOURCE_ERROR_NOT_CONNECTED = 2,
    SOURCE_ERROR_ALREADY_CONNECTED = 3,
    SOURCE_ERROR_NO_SUCH_SERVICE = 4,
    SOURCE_ERROR_NO_SUCH_CHARACTERISTIC = 5,
} SourceError;

typedef struct Source_C Source_C;

typedef SourceError (*Source_C_connect_fn)(Source_C* self);
typedef SourceError (*Source_C_disconnect_fn)(Source_C* self);
typedef SourceError (*Source_C_sendCommand_fn)(
    Source_C* self,
    const JkBmsDataBuffer_C command,
    const UuidStr service_uuid,
    const UuidStr char_uuid
);
typedef SourceError (*Source_C_subscribe_fn)(
    Source_C* self,
    const UuidStr service_uuid,
    const UuidStr char_uuid,
    void* context,
    void(*callback)(void* context, const JkBmsDataBuffer_C data)
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
        const JkBmsDataBuffer_C command,
        const UuidStr service_uuid,
        const UuidStr char_uuid
    );
    SourceError (*subscribe)(
        Source_C* self,
        const UuidStr service_uuid,
        const UuidStr char_uuid,
        void* context,
        void(*callback)(void* context, const JkBmsDataBuffer_C data)
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
    const JkBmsDataBuffer_C command,
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
    void(*callback)(void* context, const JkBmsDataBuffer_C data)
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

bool checkFrameStart(const JkBmsDataBuffer_C buffer);
bool checkFrameChecksum(const JkBmsDataBuffer_C buffer);
uint8_t calculateCRC(const JkBmsDataBuffer_C buffer);
ControllerError_C prepareCommandBuffer(uint8_t reg, 
    uint32_t value,
    uint8_t length,
    const JkBmsDataBuffer_C outBuffer);

#ifdef __cplusplus
}
#endif
