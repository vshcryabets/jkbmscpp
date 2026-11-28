#pragma once

#include "BleRepo.h"
#include <vector>
#include <string>

class GetAdaptersUseCase {
public:
    struct Result {
        BleAdapterId adapterId;
        std::string name;
    };

    virtual ~GetAdaptersUseCase() = default;
    virtual std::vector<Result> get() = 0;
};

class GetAdaptersUseCaseImpl : public GetAdaptersUseCase {
private:
    BleRepo& bleRepo;

public:
    GetAdaptersUseCaseImpl(BleRepo& repo) : bleRepo(repo) {}
    virtual ~GetAdaptersUseCaseImpl() = default;

    virtual std::vector<Result> get() override {
        std::vector<Result> results;
        auto adapterIds = bleRepo.getAdapters();
        
        for (const auto& adapterId : adapterIds) {
            Result result{
                .adapterId = adapterId,
                .name = bleRepo.getAdapterName(adapterId)
            };
            results.push_back(result);
        }
        
        return results;
    }
};

