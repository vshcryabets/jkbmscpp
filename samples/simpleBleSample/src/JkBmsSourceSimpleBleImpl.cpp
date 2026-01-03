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

JkBmsCpp::JkBmsSourceError JkBmsSourceSimpleBleImpl::connect() {
    std::string service_uuid = "0000ffe0-0000-1000-8000-00805f9b34fb"; 
    std::string char_uuid    = "0000ffe1-0000-1000-8000-00805f9b34fb";

    peripheral.connect();
    std::cout << "TA mtu=" << peripheral.mtu() << std::endl;

    peripheral.notify(service_uuid, char_uuid, [&](SimpleBLE::ByteArray bytes) {
        std::cout << "Received " << bytes.size() << " bytes: ";
        for (auto byte : bytes) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') 
                    << (int)(unsigned char)byte << " ";
        }
        std::cout << std::dec << std::endl;
    });

    // list services for debug purposes
    auto services = peripheral.services();
    for (auto& service : services) {
        std::cout << "Service: " << service.uuid() << std::endl;
        auto characteristics = service.characteristics();
        for (auto& characteristic : characteristics) {
            std::cout << "\tCharacteristic: " << characteristic.uuid() << std::endl;
        }
    }

    uint8_t frame[20];
    uint8_t length = 0;
    uint32_t value = 0;
    frame[0] = 0xAA;     // start sequence
    frame[1] = 0x55;     // start sequence
    frame[2] = 0x90;     // start sequence
    frame[3] = 0xEB;     // start sequence
    frame[4] = 0x97;  // holding register
    frame[5] = length;   // size of the value in byte
    frame[6] = value >> 0;
    frame[7] = value >> 8;
    frame[8] = value >> 16;
    frame[9] = value >> 24;
    frame[10] = 0x00;
    frame[11] = 0x00;
    frame[12] = 0x00;
    frame[13] = 0x00;
    frame[14] = 0x00;
    frame[15] = 0x00;
    frame[16] = 0x00;
    frame[17] = 0x00;
    frame[18] = 0x00;
    uint8_t crc = 0;
    for (size_t i = 0; i < sizeof(frame) - 1; i++) {
        crc += frame[i];
    }
    frame[19] = crc;

    SimpleBLE::ByteArray command_bytes(frame, sizeof(frame));

    peripheral.write_request(service_uuid, char_uuid, command_bytes);

    return JkBmsCpp::JkBmsSourceError::SUCCESS;
}

JkBmsCpp::JkBmsSourceError JkBmsSourceSimpleBleImpl::disconnect() {
    peripheral.disconnect();
    return JkBmsCpp::JkBmsSourceError::SUCCESS;
}