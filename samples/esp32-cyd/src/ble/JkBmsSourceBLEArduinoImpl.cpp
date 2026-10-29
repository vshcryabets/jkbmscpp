#include "ble/JkBmsSourceBLEArduinoImpl.h"

#include "BLEDevice.h"

JkBmsSourceBLEArduinoImpl::JkBmsSourceBLEArduinoImpl() {
    client = BLEDevice::createClient();
}

JkBmsSourceBLEArduinoImpl::~JkBmsSourceBLEArduinoImpl() {
    disconnect();
}

SourceError JkBmsSourceBLEArduinoImpl::connect() {
    if (client->isConnected()) {
        return SOURCE_ERROR_ALREADY_CONNECTED;
    }
    return client->connect(BLEAddress(address)) ? SOURCE_ERROR_SUCCESS : SOURCE_ERROR_FAILURE;
}

SourceError JkBmsSourceBLEArduinoImpl::disconnect() {
    if (!client->isConnected()) {
        return SOURCE_ERROR_NOT_CONNECTED;
    }
    client->disconnect();
    return SOURCE_ERROR_SUCCESS;
}

SourceError JkBmsSourceBLEArduinoImpl::sendCommand(
    const JkBmsCpp::JkBmsDataBuffer_C command,
    const JkBmsCpp::UuidStr service_uuid,
    const JkBmsCpp::UuidStr char_uuid
) {
    client->setValue(
        BLEUUID(
            const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(service_uuid)),
            36, 
            true
        ),
        BLEUUID(
            const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(char_uuid)),
            36, 
            true
        ),
        std::string(
            (char*)command.ptr, 
            command.len
        )
    );
    return SOURCE_ERROR_SUCCESS;
}

SourceError JkBmsSourceBLEArduinoImpl::subscribe(
    const JkBmsCpp::UuidStr service_uuid,
    const JkBmsCpp::UuidStr char_uuid,
    void* context,
    void(*callback)(void* context, const JkBmsCpp::JkBmsDataBuffer_C &data)
) {
    BLERemoteService* service = client->getService(
        BLEUUID(
            const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(service_uuid)),
            36, 
            true
        )
    );
    if (service == nullptr) {
        return SOURCE_ERROR_NO_SUCH_SERVICE;
    }
    BLERemoteCharacteristic* characteristic = service->getCharacteristic(
        BLEUUID(
            const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(char_uuid)),
            36, 
            true
        )
    );
    if (characteristic == nullptr) {
        return SOURCE_ERROR_NO_SUCH_CHARACTERISTIC;
    }
    characteristic->registerForNotify(
        [context, callback](
            BLERemoteCharacteristic* pBLERemoteCharacteristic, 
            uint8_t* pData, 
            size_t length, 
            bool isNotify) {
            JkBmsCpp::JkBmsDataBuffer_C dataBuffer = {pData, (uint16_t)length};
            callback(context, dataBuffer);
        }
    );
    return SOURCE_ERROR_SUCCESS;
}

SourceError JkBmsSourceBLEArduinoImpl::unsubscribe(
    const JkBmsCpp::UuidStr service_uuid,
    const JkBmsCpp::UuidStr char_uuid
) {
    BLERemoteService* service = client->getService(
        BLEUUID(
            const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(service_uuid)),
            36, 
            true
        )
    );
    if (service == nullptr) {
        return SOURCE_ERROR_NO_SUCH_SERVICE;
    }
    BLERemoteCharacteristic* characteristic = service->getCharacteristic(
        BLEUUID(
            const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(char_uuid)), 
            36, 
            true
        )
    );
    if (characteristic == nullptr) {
        return SOURCE_ERROR_NO_SUCH_CHARACTERISTIC;
    }
    characteristic->registerForNotify(nullptr);
    return SOURCE_ERROR_SUCCESS; 
}

void JkBmsSourceBLEArduinoImpl::setAddress(uint8_t address[6]) {
    memcpy(this->address, address, sizeof(this->address));
}

size_t JkBmsSourceBLEArduinoImpl::getMtu() {
    return client->getMTU();   
}