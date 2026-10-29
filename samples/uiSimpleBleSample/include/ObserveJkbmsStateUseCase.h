#pragma once

#include <string>
#include "stateflow.h"
#include "BleRepo.h"

namespace JkBmsCpp {

    class ObserveJkbmsStateUseCase {
    public:
        virtual ~ObserveJkbmsStateUseCase() = default;
        virtual flow::StateFlow<JkbmsState>& observe(const std::string& peripheralId) = 0;
    };

    class ObserveJkbmsStateUseCaseImpl : public ObserveJkbmsStateUseCase {
    private:
        BleRepo& bleRepo;

    public:
        ObserveJkbmsStateUseCaseImpl(BleRepo& repo) : bleRepo(repo) {}
        virtual ~ObserveJkbmsStateUseCaseImpl() = default;

        virtual flow::StateFlow<JkbmsState>& observe(const std::string& peripheralId) override {
            return *bleRepo.getBmsState(0, peripheralId);
        }
    };

}

