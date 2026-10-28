#pragma once

#include <string>
#include "stateflow.h"

namespace JkBmsCpp {

    struct JkbmsState {
        std::string vendorId;
        std::string hwVersion;
        std::string swVersion;
        uint32_t uptimeSeconds;
        std::string deviceName;
        std::string serialNumber;

        bool operator==(const JkbmsState& other) const {
            return vendorId == other.vendorId &&
                   hwVersion == other.hwVersion &&
                   swVersion == other.swVersion &&
                   uptimeSeconds == other.uptimeSeconds &&
                   deviceName == other.deviceName &&
                   serialNumber == other.serialNumber;
        }

        bool operator!=(const JkbmsState& other) const {
            return !(*this == other);
        }
    };

    class ObserveJkbmsStateUseCase {
    public:
        virtual ~ObserveJkbmsStateUseCase() = default;
        virtual flow::StateFlow<JkbmsState> observe() = 0;
    };

}

