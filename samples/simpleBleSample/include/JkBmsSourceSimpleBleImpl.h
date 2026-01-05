#pragma once 
#include "JkBmsController.h"
#include <simpleble/SimpleBLE.h>


class JkBmsSourceSimpleBleImpl : public JkBmsCpp::JkBmsSource {
    private:
        SimpleBLE::Peripheral peripheral;
    public:
        JkBmsSourceSimpleBleImpl(const SimpleBLE::Peripheral &peripheral);
        virtual ~JkBmsSourceSimpleBleImpl();

        virtual JkBmsCpp::JkBmsSourceError connect() override;
        virtual JkBmsCpp::JkBmsSourceError disconnect() override;
        virtual JkBmsCpp::JkBmsSourceError sendCommand(
            const uint8_t* data, 
            const uint16_t size,
            const JkBmsCpp::JkBmsString& service_uuid,
            const JkBmsCpp::JkBmsString& char_uuid) override;
        virtual size_t getMtu() override;
        virtual JkBmsCpp::JkBmsSourceError subscribe(
            const JkBmsCpp::JkBmsString& service_uuid,
            const JkBmsCpp::JkBmsString& char_uuid,
            void* context,
            void(*callback)(void* context, const uint8_t* data, const uint16_t size)
        ) override;
        virtual JkBmsCpp::JkBmsSourceError unsubscribe(
            const JkBmsCpp::JkBmsString& service_uuid,
            const JkBmsCpp::JkBmsString& char_uuid
        ) override;        
};
