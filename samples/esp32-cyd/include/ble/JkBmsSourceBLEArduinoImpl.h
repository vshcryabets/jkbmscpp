#pragma once

#include <BLEClient.h>

#include "JkBmsController.h"

class JkBmsSourceDummyImpl : public JkBmsCpp::Source {
    public:
        JkBmsSourceDummyImpl() = default;
        ~JkBmsSourceDummyImpl() override = default;
        SourceError connect() override { return SOURCE_ERROR_SUCCESS; }
        SourceError disconnect() override { return SOURCE_ERROR_SUCCESS; }
        SourceError sendCommand(
            const JkBmsCpp::JkBmsDataBuffer_C command,
            const JkBmsCpp::UuidStr service_uuid,
            const JkBmsCpp::UuidStr char_uuid
        ) override { return SOURCE_ERROR_SUCCESS; }
        SourceError subscribe(
            const JkBmsCpp::UuidStr service_uuid,
            const JkBmsCpp::UuidStr char_uuid,
            void* context,
            void(*callback)(void* context, const JkBmsCpp::JkBmsDataBuffer_C &data)
        ) override { return SOURCE_ERROR_SUCCESS; }
        SourceError unsubscribe(
            const JkBmsCpp::UuidStr service_uuid,
            const JkBmsCpp::UuidStr char_uuid
        ) override { return SOURCE_ERROR_SUCCESS; }
        size_t getMtu() override { return 23; }
};


class JkBmsSourceBLEArduinoImpl : public JkBmsCpp::Source {
private:
    uint8_t address[6];
    BLEClient *client;
public:
    JkBmsSourceBLEArduinoImpl();
    ~JkBmsSourceBLEArduinoImpl() override;
    SourceError connect() override;
    SourceError disconnect() override;
    SourceError sendCommand(
        const JkBmsCpp::JkBmsDataBuffer_C command,
        const JkBmsCpp::UuidStr service_uuid,
        const JkBmsCpp::UuidStr char_uuid
    ) override;
    SourceError subscribe(
        const JkBmsCpp::UuidStr service_uuid,
        const JkBmsCpp::UuidStr char_uuid,
        void* context,
        void(*callback)(void* context, const JkBmsCpp::JkBmsDataBuffer_C &data)
    ) override;
    SourceError unsubscribe(
        const JkBmsCpp::UuidStr service_uuid,
        const JkBmsCpp::UuidStr char_uuid
    ) override;
    size_t getMtu() override;
    void setAddress(uint8_t address[6]);
};
