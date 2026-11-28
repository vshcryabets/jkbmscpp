#include <cstdint>
#include <cstdarg>

#include "JkBmsController.h"
#include "JkBmsTools.h"

namespace JkBmsCpp
{

UuidStr Controller::SERVICE_UUID = {'0','0','0','0','f','f','e','0','-','0','0','0','0','-','1','0','0','0','-','8','0','0','0','-','0','0','8','0','5','f','9','b','3','4','f','b','\0'};
UuidStr Controller::CHARACTERISTIC_UUID = {'0','0','0','0','f','f','e','1','-','0','0','0','0','-','1','0','0','0','-','8','0','0','0','-','0','0','8','0','5','f','9','b','3','4','f','b','\0'};

DumpLog Controller::dumpLogInstance;

Controller::Controller(Log& logger) : 
    source(nullptr),
#ifdef JKBMSCPP_USE_STATEFLOW
    cellInfoStateFlow(std::make_shared<flow::StateFlowImpl<CellInfo>>(CellInfo{})),
#endif    
    log(logger)
{
    responseBuffer.reserve(MAX_PACKET_SIZE);
}

Controller::~Controller()
{
}

void Controller::start(Source* source)
{
    this->source = source;
    this->source->connect();
    this->source->subscribe(
        SERVICE_UUID,
        CHARACTERISTIC_UUID,
        (void*)this,
        this->notificationCallback);
}

void Controller::end()
{
    if (this->source)
    {
        this->source->unsubscribe(
            SERVICE_UUID,
            CHARACTERISTIC_UUID);
        this->source->disconnect();
        this->source = nullptr;
    }
}

ControllerError Controller::requestCellInfoPeriodicUpdates() noexcept {
    if (cellInfoUpdatesRequested) {
        log.info("Cell info updates already requested");
        return CONTROLLER_ERROR_SUCCESS;
    }
    if (source == nullptr) {
        log.error("Source is not started");
        return CONTROLLER_ERROR_NO_SOURCE;
    }
    uint8_t frame[20];
    JkBmsDataBuffer command = {frame, sizeof(frame)};
    auto result = prepareCommandBuffer(CELLS_INFO_REGISTER, 0, 0, command);
    if (result != CONTROLLER_ERROR_SUCCESS) {
        return result;
    }
    source->sendCommand(command, SERVICE_UUID, CHARACTERISTIC_UUID);
    cellInfoUpdatesRequested = true;
    log.info("Requested periodic cell info updates");
    return CONTROLLER_ERROR_SUCCESS;
}

CellInfoFuture Controller::readCellsState() {
    pendingCellInfoRequest = std::make_unique<CellInfoPromise>();
    auto result = requestCellInfoPeriodicUpdates();
    if (result != CONTROLLER_ERROR_SUCCESS) {
        pendingCellInfoRequest->set_value(
            Expected<CellInfoResponse, ControllerError>(
                result));
        return pendingCellInfoRequest->get_future();
    }
    return pendingCellInfoRequest->get_future();
}

DeviceInfoFuture Controller::readDeviceState()
{
    pendingDeviceInfoRequest = std::make_unique<DeviceInfoPromise>();
    if (source == nullptr) {
        log.error("Source is not started");
        pendingDeviceInfoRequest->set_value(
            Expected<DeviceInfoResponse, ControllerError>(
                CONTROLLER_ERROR_NO_SOURCE));
        return pendingDeviceInfoRequest->get_future();
    }
    uint8_t frame[20];
    JkBmsDataBuffer command = {frame, sizeof(frame)};
    prepareCommandBuffer(DEVICE_INFO_REGISTER, 0, 0, command);
    source->sendCommand(command, SERVICE_UUID, CHARACTERISTIC_UUID);
    return pendingDeviceInfoRequest->get_future();
}

void Controller::notificationCallback(
            void* ctx,
            const JkBmsDataBuffer data) {
    Controller* controller = static_cast<Controller*>(ctx);
    controller->handleResponse(data);
}

void Controller::handleResponse(const JkBmsDataBuffer data) noexcept {
    try {
    log.info("Received %u bytes", data.len);
    dumpDataToLog(data.ptr, data.len, log);
    if (checkFrameStart(data)) {
        // we received new packet, clear previous data
        log.info("New packet detected, clearing buffer");
        responseBuffer.clear();
    }
    responseBuffer.insert(responseBuffer.end(), data.ptr, data.ptr + data.len);
    if (responseBuffer.size() >= DEFAULT_PACKET_SIZE) {
        // we have full packet, process it
        JkBmsDataBuffer buffer = {responseBuffer.data(), (uint16_t)responseBuffer.size()};
        if (!checkFrameChecksum(buffer)) {
            log.error("Checksum mismatch");
            return;
        }
        auto responseType = getResponseType(buffer);
        if (!responseType.has_value()) {
            log.error("Error getting response type: %d", static_cast<uint8_t>(responseType.error()));
            return;
        } else {
            log.info("Response type: %d", static_cast<int>(responseType.value()));
            switch (responseType.value())
            {
            case ResponseType::DEVICE_INFO:
                {
                    JkBmsDataBuffer deviceBuffer = {responseBuffer.data(), (uint16_t)responseBuffer.size()};
                    handleDeviceInfoResponse(deviceBuffer);
                }
                break;
            case ResponseType::CELL_INFO:
                {
                    JkBmsDataBuffer cellBuffer = {responseBuffer.data(), (uint16_t)responseBuffer.size()};
                    handleCellInfoResponse(cellBuffer);
                }
                break;
            default:
                break;
            }
            responseBuffer.clear();
        }
    }
    } catch (const std::exception& e) {
        log.error("Exception in handleResponse: %s: %s", typeid(e).name(), e.what());
    } catch (...) {
        log.error("Unknown exception in handleResponse");
    }
}

void Controller::handleDeviceInfoResponse(const ::JkBmsDataBuffer data) noexcept
{
    auto deviceInfo = parseDeviceInfo(data);
    if (pendingDeviceInfoRequest) {                        
        pendingDeviceInfoRequest->set_value(deviceInfo);
        pendingDeviceInfoRequest.reset();
    }
}

void Controller::handleCellInfoResponse(const JkBmsDataBuffer data) noexcept
{
    auto cellInfo = parseCellsInfo(data, JK_BMS_STRUCTURE_CELLS_COUNT);
    
    if (pendingCellInfoRequest) {
        pendingCellInfoRequest->set_value(cellInfo);
        pendingCellInfoRequest.reset();
    }

#ifdef JKBMSCPP_USE_STATEFLOW
    if (cellInfo.has_value()) {
        cellInfoStateFlow->emit(CellInfo(cellInfo.value()));
    }
#endif
}

#ifdef JKBMSCPP_USE_STATEFLOW
std::shared_ptr<flow::StateFlow<CellInfo>> Controller::observeCellInfoState() {
    return cellInfoStateFlow;
}
#endif

} // namespace JkBmsCpp