#include "JkBmsSourceSimpleBleImpl.h"
#include <iostream>
#include <chrono>
#include <thread>
#include "JkBmsTools.h"

JkBmsSourceSimpleBleImpl::JkBmsSourceSimpleBleImpl(const SimpleBLE::Peripheral &peripheral)
    : peripheral(peripheral) {
}

JkBmsSourceSimpleBleImpl::~JkBmsSourceSimpleBleImpl() {
    disconnect();
}

JkBmsCpp::SourceError JkBmsSourceSimpleBleImpl::sendCommand(
    const JkBmsCpp::JkBmsDataBuffer& command,
    const JkBmsCpp::JkBmsString& service_uuid,
    const JkBmsCpp::JkBmsString& char_uuid) {
    SimpleBLE::ByteArray command_bytes(command.data(), command.size());
    std::cout << "Sending command (" << command.size() << " bytes):" << std::endl;
    JkBmsCpp::dumpDataToLog(command.data(), command.size());
    peripheral.write_request(service_uuid, char_uuid, command_bytes);
    return JkBmsCpp::SourceError::SUCCESS;
}

JkBmsCpp::SourceError JkBmsSourceSimpleBleImpl::connect() {
    peripheral.connect();
    return JkBmsCpp::SourceError::SUCCESS;
}

JkBmsCpp::SourceError JkBmsSourceSimpleBleImpl::disconnect() {
    peripheral.disconnect();
    return JkBmsCpp::SourceError::SUCCESS;
}

size_t JkBmsSourceSimpleBleImpl::getMtu() {
    return peripheral.mtu();
}

JkBmsCpp::SourceError JkBmsSourceSimpleBleImpl::subscribe(
    const JkBmsCpp::JkBmsString& service_uuid,
    const JkBmsCpp::JkBmsString& char_uuid,
    void* context,
    void(*callback)(void* context, const JkBmsCpp::JkBmsDataBuffer& data)
) {
    peripheral.notify(service_uuid, 
        char_uuid, [callback, context](SimpleBLE::ByteArray bytes) {
            callback(
                context, 
                JkBmsCpp::JkBmsDataBuffer(
                    bytes.data(),
                    static_cast<uint16_t>(bytes.size())
                ));
    });
    return JkBmsCpp::SourceError::SUCCESS;
}

JkBmsCpp::SourceError JkBmsSourceSimpleBleImpl::unsubscribe(
    const JkBmsCpp::JkBmsString& service_uuid,
    const JkBmsCpp::JkBmsString& char_uuid
) {
    peripheral.unsubscribe(service_uuid, char_uuid);
    return JkBmsCpp::SourceError::SUCCESS;
}