#pragma once

#include <string>
#include "stateflow.h"
#include "BleRepo.h"

namespace JkBmsCpp {

    class ObserveJkbmsStateUseCase {
    public:
        virtual ~ObserveJkbmsStateUseCase() = default;
        virtual flow::StateFlow<JkbmsState> observe(const std::string& peripheralId) = 0;
    };

}

