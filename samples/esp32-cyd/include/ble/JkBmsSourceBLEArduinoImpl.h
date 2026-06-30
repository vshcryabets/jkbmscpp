#pragma once

#include "JkBmsController.h"


class JkBmsSourceBLEArduinoImpl : public JkBmsCpp::Source {
private:
    uint8_t address_[6];
public:
    JkBmsSourceBLEArduinoImpl(uint8_t address[6]);
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
};
