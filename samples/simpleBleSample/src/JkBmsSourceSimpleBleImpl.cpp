#include "JkBmsSourceSimpleBleImpl.h"
#include <iostream>
#include <chrono>
#include <thread>
#include "JkBmsTools.h"

JkBmsSourceSimpleBleImpl::JkBmsSourceSimpleBleImpl(const SimpleBLE::Peripheral &peripheral, JkBmsCpp::Log& logger)
    : peripheral(peripheral), log(logger) {
}

JkBmsSourceSimpleBleImpl::~JkBmsSourceSimpleBleImpl() {
    disconnect();
}

SourceError JkBmsSourceSimpleBleImpl::sendCommand(
    const JkBmsCpp::JkBmsDataBuffer_C command,
    const JkBmsCpp::UuidStr service_uuid,
    const JkBmsCpp::UuidStr char_uuid) {
    SimpleBLE::ByteArray command_bytes(command.ptr, command.ptr + command.len);
    log.info("Sending command (%u bytes):", command.len);
    JkBmsCpp::dumpDataToLog(command.ptr, command.len, log);
    peripheral.write_request(service_uuid, char_uuid, command_bytes);
    return SOURCE_ERROR_SUCCESS;
}

SourceError JkBmsSourceSimpleBleImpl::connect() {
    peripheral.connect();
    return SOURCE_ERROR_SUCCESS;
}

SourceError JkBmsSourceSimpleBleImpl::disconnect() {
    peripheral.disconnect();
    return SOURCE_ERROR_SUCCESS;
}

size_t JkBmsSourceSimpleBleImpl::getMtu() {
    return peripheral.mtu();
}

SourceError JkBmsSourceSimpleBleImpl::subscribe(
    const JkBmsCpp::UuidStr service_uuid,
    const JkBmsCpp::UuidStr char_uuid,
    void* context,
    void(*callback)(void* context, const JkBmsCpp::JkBmsDataBuffer_C data)
) {
    peripheral.notify(service_uuid, 
        char_uuid, [callback, context](SimpleBLE::ByteArray bytes) {
            JkBmsCpp::JkBmsDataBuffer_C buffer;
            buffer.ptr = bytes.data();
            buffer.len = static_cast<uint16_t>(bytes.size());
            callback(context, buffer);
    });
    return SOURCE_ERROR_SUCCESS;
}

SourceError JkBmsSourceSimpleBleImpl::unsubscribe(
    const JkBmsCpp::UuidStr service_uuid,
    const JkBmsCpp::UuidStr char_uuid
) {
    peripheral.unsubscribe(service_uuid, char_uuid);
    return SOURCE_ERROR_SUCCESS;
}