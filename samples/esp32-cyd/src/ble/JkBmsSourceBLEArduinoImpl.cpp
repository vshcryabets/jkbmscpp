#include "ble/JkBmsSourceBLEArduinoImpl.h"

#include "BLEDevice.h"

JkBmsSourceBLEArduinoImpl::JkBmsSourceBLEArduinoImpl() {
    client = BLEDevice::createClient();
}

JkBmsSourceBLEArduinoImpl::~JkBmsSourceBLEArduinoImpl() {
    disconnect();
}

JkBmsCpp::SourceError JkBmsSourceBLEArduinoImpl::connect() {
    if (client->isConnected()) {
        return JkBmsCpp::SourceError::ALREADY_CONNECTED;
    }
    return client->connect(BLEAddress(address)) ? JkBmsCpp::SourceError::SUCCESS : JkBmsCpp::SourceError::FAILURE;
}

JkBmsCpp::SourceError JkBmsSourceBLEArduinoImpl::disconnect() {
    if (!client->isConnected()) {
        return JkBmsCpp::SourceError::NOT_CONNECTED;
    }
    client->disconnect();
    return JkBmsCpp::SourceError::SUCCESS;
}

JkBmsCpp::SourceError JkBmsSourceBLEArduinoImpl::sendCommand(
    const JkBmsCpp::JkBmsDataBuffer& command,
    const JkBmsCpp::JkBmsString& service_uuid,
    const JkBmsCpp::JkBmsString& char_uuid
) {
    client->setValue(
        // BLEUUID(service_uuid.data, service_uuid.length, true),
        BLEUUID(
            const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(service_uuid.data)),
            service_uuid.length, 
            true
        ),
        BLEUUID(
            const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(char_uuid.data)),
            char_uuid.length, 
            true
        ),
        // BLEUUID(char_uuid.data, char_uuid.length, true), 
        std::string(
            (char*)command.data(), 
            command.size()
        )
    );
    return JkBmsCpp::SourceError::SUCCESS;
}

JkBmsCpp::SourceError JkBmsSourceBLEArduinoImpl::subscribe(
    const JkBmsCpp::JkBmsString& service_uuid,
    const JkBmsCpp::JkBmsString& char_uuid,
    void* context,
    void(*callback)(void* context, const JkBmsCpp::JkBmsDataBuffer &data)
) {
    BLERemoteService* service = client->getService(
        BLEUUID(
            const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(service_uuid.data)),
            service_uuid.length, 
            true
        )
    );
    if (service == nullptr) {
        return JkBmsCpp::SourceError::NO_SUCH_SERVICE;
    }
    BLERemoteCharacteristic* characteristic = service->getCharacteristic(
        BLEUUID(
            const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(char_uuid.data)),
            char_uuid.length, 
            true
        )
    );
    if (characteristic == nullptr) {
        return JkBmsCpp::SourceError::NO_SUCH_SERVICE;
    }
    characteristic->registerForNotify(
        [context, callback](
            BLERemoteCharacteristic* pBLERemoteCharacteristic, 
            uint8_t* pData, 
            size_t length, 
            bool isNotify) {
            JkBmsCpp::JkBmsDataBuffer dataBuffer(pData, length);
            callback(context, dataBuffer);
        }
    );
    return JkBmsCpp::SourceError::SUCCESS;
}

JkBmsCpp::SourceError JkBmsSourceBLEArduinoImpl::unsubscribe(
    const JkBmsCpp::JkBmsString& service_uuid,
    const JkBmsCpp::JkBmsString& char_uuid
) {
    BLERemoteService* service = client->getService(
        BLEUUID(
            const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(service_uuid.data)),
            service_uuid.length, 
            true
        )
    );
    if (service == nullptr) {
        return JkBmsCpp::SourceError::NO_SUCH_SERVICE;
    }
    BLERemoteCharacteristic* characteristic = service->getCharacteristic(
        BLEUUID(
            const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(char_uuid.data)), 
            char_uuid.length, 
            true
        )
    );
    if (characteristic == nullptr) {
        return JkBmsCpp::SourceError::NO_SUCH_SERVICE;
    }
    characteristic->registerForNotify(nullptr);
    return JkBmsCpp::SourceError::SUCCESS; 
}

void JkBmsSourceBLEArduinoImpl::setAddress(uint8_t address[6]) {
    memcpy(this->address, address, sizeof(this->address));
}

size_t JkBmsSourceBLEArduinoImpl::getMtu() {
    return client->getMTU();   
}