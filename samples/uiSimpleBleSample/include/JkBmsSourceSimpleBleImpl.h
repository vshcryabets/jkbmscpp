#pragma once 
#include "JkBmsController.h"
#include <simpleble/SimpleBLE.h>


class JkBmsSourceSimpleBleImpl : public JkBmsCpp::Source {
    private:
        SimpleBLE::Peripheral peripheral;
        JkBmsCpp::Log& log;
    public:
        JkBmsSourceSimpleBleImpl(const SimpleBLE::Peripheral &peripheral, JkBmsCpp::Log& logger);
        virtual ~JkBmsSourceSimpleBleImpl();

        virtual SourceError connect() override;
        virtual SourceError disconnect() override;
        virtual SourceError sendCommand(
            const JkBmsCpp::JkBmsDataBuffer_C command,
            const JkBmsCpp::UuidStr service_uuid,
            const JkBmsCpp::UuidStr char_uuid) override;
        virtual size_t getMtu() override;
        virtual SourceError subscribe(
            const JkBmsCpp::UuidStr service_uuid,
            const JkBmsCpp::UuidStr char_uuid,
            void* context,
            void(*callback)(void* context, const JkBmsCpp::JkBmsDataBuffer_C data)
        ) override;
        virtual SourceError unsubscribe(
            const JkBmsCpp::UuidStr service_uuid,
            const JkBmsCpp::UuidStr char_uuid
        ) override;        
};
