#include "JkBmsSourceSimpleBleImpl.h"
#include <iostream>

JkBmsSourceSimpleBleImpl::JkBmsSourceSimpleBleImpl(const SimpleBLE::Peripheral &peripheral)
    : peripheral(peripheral) {
}

JkBmsSourceSimpleBleImpl::~JkBmsSourceSimpleBleImpl() {
    disconnect();
}

JkBmsCpp::JkBmsSourceError JkBmsSourceSimpleBleImpl::connect() {
    peripheral.connect();

    // list services for debug purposes
    auto services = peripheral.services();
    for (auto& service : services) {
        std::cout << "Service: " << service.uuid() << std::endl;
        auto characteristics = service.characteristics();
        for (auto& characteristic : characteristics) {
            std::cout << "\tCharacteristic: " << characteristic.uuid() << std::endl;
        }
    }
    
    return JkBmsCpp::JkBmsSourceError::SUCCESS;
}

JkBmsCpp::JkBmsSourceError JkBmsSourceSimpleBleImpl::disconnect() {
    peripheral.disconnect();
    return JkBmsCpp::JkBmsSourceError::SUCCESS;
}