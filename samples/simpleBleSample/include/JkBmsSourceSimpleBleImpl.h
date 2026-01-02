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
};
