#include <iostream>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include "app.h"
#include "BleRepoSimpleBleImpl.h"
#include "IoExecutor.h"
#include "GetAdaptersUseCase.h"
#include "StartScanUseCase.h"
#include "StopScanUseCase.h"
#include "ObserveScanResultsUseCase.h"
#include "StartConnectionUseCase.h"
#include "StopConnectionUseCase.h"
#include "ObserveJkbmsStateUseCase.h"

void init_logger() {
    try {
        auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            "logs/uisample.log", 1024 * 1024, 5
        );
        rotating_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [T%t] %v");
        std::vector<spdlog::sink_ptr> sinks { rotating_sink };
        auto logger = std::make_shared<spdlog::logger>("multi_sink_logger", sinks.begin(), sinks.end());
        logger->set_level(spdlog::level::trace);
        spdlog::flush_every(std::chrono::seconds(3));
        spdlog::set_default_logger(logger);
    } catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "Log initialization failed: " << ex.what() << std::endl;
    }
}

int main() {
    init_logger();
    
    BleRepoSimpleBleImpl bleRepo;
    auto getAdaptersUseCase = std::make_unique<GetAdaptersUseCaseImpl>(bleRepo);
    auto startScanUseCase = std::make_unique<StartScanUseCaseImpl>(bleRepo);
    auto stopScanUseCase = std::make_unique<StopScanUseCaseImpl>(bleRepo);
    auto observeScanResultsUseCase = std::make_unique<ObserveScanResultsUseCaseImpl>(bleRepo);
    auto startConnectionUseCase = std::make_unique<StartConnectionUseCaseImpl>(bleRepo);
    auto stopConnectionUseCase = std::make_unique<StopConnectionUseCaseImpl>(bleRepo);
    auto observeJkbmsStateUseCase = std::make_unique<JkBmsCpp::ObserveJkbmsStateUseCaseImpl>(bleRepo);
    
    uiSimpleBleSample::IoExecutor ioExecutor(2);

    uiSimpleBleSample::ViewModel viewModel(*getAdaptersUseCase, *startScanUseCase, *stopScanUseCase, *observeScanResultsUseCase, *startConnectionUseCase, *stopConnectionUseCase, *observeJkbmsStateUseCase, ioExecutor);
    uiSimpleBleSample::App app(viewModel);
    app.run();
    return 0;
}