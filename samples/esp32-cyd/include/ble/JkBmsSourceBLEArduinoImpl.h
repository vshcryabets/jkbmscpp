#pragma once

#include <BLEClient.h>

#include "JkBmsController.h"

class JkBmsSourceDummyImpl : public JkBmsCpp::Source {
    public:
        JkBmsSourceDummyImpl() = default;
        ~JkBmsSourceDummyImpl() override = default;
        JkBmsCpp::SourceError connect() override { return JkBmsCpp::SourceError::SUCCESS; }
        JkBmsCpp::SourceError disconnect() override { return JkBmsCpp::SourceError::SUCCESS; }
        JkBmsCpp::SourceError sendCommand(
            const JkBmsCpp::JkBmsDataBuffer& command,
            const JkBmsCpp::JkBmsString& service_uuid,
            const JkBmsCpp::JkBmsString& char_uuid
        ) override { return JkBmsCpp::SourceError::SUCCESS; }
        JkBmsCpp::SourceError subscribe(
            const JkBmsCpp::JkBmsString& service_uuid,
            const JkBmsCpp::JkBmsString& char_uuid,
            void* context,
            void(*callback)(void* context, const JkBmsCpp::JkBmsDataBuffer &data)
        ) override { return JkBmsCpp::SourceError::SUCCESS; }
        JkBmsCpp::SourceError unsubscribe(
            const JkBmsCpp::JkBmsString& service_uuid,
            const JkBmsCpp::JkBmsString& char_uuid
        ) override { return JkBmsCpp::SourceError::SUCCESS; }
        size_t getMtu() override { return 23; }
};


class JkBmsSourceBLEArduinoImpl : public JkBmsCpp::Source {
private:
    uint8_t address[6];
    BLEClient *client;
public:
    JkBmsSourceBLEArduinoImpl();
    ~JkBmsSourceBLEArduinoImpl() override;
    JkBmsCpp::SourceError connect() override;
    JkBmsCpp::SourceError disconnect() override;
    JkBmsCpp::SourceError sendCommand(
        const JkBmsCpp::JkBmsDataBuffer& command,
        const JkBmsCpp::JkBmsString& service_uuid,
        const JkBmsCpp::JkBmsString& char_uuid
    ) override;
    JkBmsCpp::SourceError subscribe(
        const JkBmsCpp::JkBmsString& service_uuid,
        const JkBmsCpp::JkBmsString& char_uuid,
        void* context,
        void(*callback)(void* context, const JkBmsCpp::JkBmsDataBuffer &data)
    ) override;
    JkBmsCpp::SourceError unsubscribe(
        const JkBmsCpp::JkBmsString& service_uuid,
        const JkBmsCpp::JkBmsString& char_uuid
    ) override;
    size_t getMtu() override;
    void setAddress(uint8_t address[6]);
};
