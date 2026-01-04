#include "JkBmsSourceSimpleBleImpl.h"
#include <iostream>
#include <chrono>
#include <thread>

JkBmsSourceSimpleBleImpl::JkBmsSourceSimpleBleImpl(const SimpleBLE::Peripheral &peripheral)
    : peripheral(peripheral) {
}

JkBmsSourceSimpleBleImpl::~JkBmsSourceSimpleBleImpl() {
    disconnect();
}

JkBmsCpp::JkBmsSourceError JkBmsSourceSimpleBleImpl::sendCommand(
    const uint8_t* data, 
    const uint16_t size,
    const JkBmsCpp::JkBmsString& service_uuid,
    const JkBmsCpp::JkBmsString& char_uuid) {
    SimpleBLE::ByteArray command_bytes(data, size);
    peripheral.write_request(service_uuid, char_uuid, command_bytes);
    return JkBmsCpp::JkBmsSourceError::SUCCESS;
}

JkBmsCpp::JkBmsSourceError JkBmsSourceSimpleBleImpl::connect() {
    peripheral.connect();



    // // list services for debug purposes
    // auto services = peripheral.services();
    // for (auto& service : services) {
    //     std::cout << "Service: " << service.uuid() << std::endl;
    //     auto characteristics = service.characteristics();
    //     for (auto& characteristic : characteristics) {
    //         std::cout << "\tCharacteristic: " << characteristic.uuid() << std::endl;
    //     }
    // }
    return JkBmsCpp::JkBmsSourceError::SUCCESS;
}

JkBmsCpp::JkBmsSourceError JkBmsSourceSimpleBleImpl::disconnect() {
    peripheral.disconnect();
    return JkBmsCpp::JkBmsSourceError::SUCCESS;
}

size_t JkBmsSourceSimpleBleImpl::getMtu() {
    return peripheral.mtu();
}

JkBmsCpp::JkBmsSourceError JkBmsSourceSimpleBleImpl::subscribe(
    const JkBmsCpp::JkBmsString& service_uuid,
    const JkBmsCpp::JkBmsString& char_uuid,
    const void* context,
    void(*callback)(const void* context, const uint8_t* data, const uint16_t size)
) {
    peripheral.notify(service_uuid, 
        char_uuid, [callback, context](SimpleBLE::ByteArray bytes) {
            callback(context, bytes.data(), static_cast<uint16_t>(bytes.size()));
    });
    return JkBmsCpp::JkBmsSourceError::SUCCESS;
}

JkBmsCpp::JkBmsSourceError JkBmsSourceSimpleBleImpl::unsubscribe(
    const JkBmsCpp::JkBmsString& service_uuid,
    const JkBmsCpp::JkBmsString& char_uuid
) {
    peripheral.unsubscribe(service_uuid, char_uuid);
    return JkBmsCpp::JkBmsSourceError::SUCCESS;
}