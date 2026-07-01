#pragma once 
#include "JkBmsController.h"
#include <simpleble/SimpleBLE.h>


class JkBmsSourceSimpleBleImpl : public JkBmsCpp::Source {
    private:
        SimpleBLE::Peripheral peripheral;
    public:
        JkBmsSourceSimpleBleImpl(const SimpleBLE::Peripheral &peripheral);
        virtual ~JkBmsSourceSimpleBleImpl();

        virtual JkBmsCpp::SourceError connect() override;
        virtual JkBmsCpp::SourceError disconnect() override;
        virtual JkBmsCpp::SourceError sendCommand(
            const JkBmsCpp::JkBmsDataBuffer& command,
            const JkBmsCpp::JkBmsString& service_uuid,
            const JkBmsCpp::JkBmsString& char_uuid) override;
        virtual size_t getMtu() override;
        virtual JkBmsCpp::SourceError subscribe(
            const JkBmsCpp::JkBmsString& service_uuid,
            const JkBmsCpp::JkBmsString& char_uuid,
            void* context,
            void(*callback)(void* context, const JkBmsCpp::JkBmsDataBuffer& data)
        ) override;
        virtual JkBmsCpp::SourceError unsubscribe(
            const JkBmsCpp::JkBmsString& service_uuid,
            const JkBmsCpp::JkBmsString& char_uuid
        ) override;        
};
