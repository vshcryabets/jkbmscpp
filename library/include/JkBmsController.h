#pragma once

#include <cstdint>
#include <future>
#include <memory>
#include <atomic>

#include "JkBmsFrames.h"
#include "CppWrappers.h"
#include "CFunctions.h"

#ifdef JKBMSCPP_USE_STATEFLOW
#include "stateflow.h"
#endif

namespace JkBmsCpp {
    constexpr static uint8_t DEVICE_INFO_REGISTER = 0x97;
    constexpr static uint8_t CELLS_INFO_REGISTER = 0x96;

    typedef std::future<Expected<DeviceInfoResponse, ControllerError>> DeviceInfoFuture;
    typedef std::promise<Expected<DeviceInfoResponse, ControllerError>> DeviceInfoPromise;

    typedef std::future<Expected<CellInfoResponse, ControllerError>> CellInfoFuture;
    typedef std::promise<Expected<CellInfoResponse, ControllerError>> CellInfoPromise;

    class CellInfoStateListener {
    public:
        virtual ~CellInfoStateListener() = default;
        virtual void onCellInfoStateChanged(const CellInfoResponse& response) = 0;
    };

    class Source {
    public:
        virtual ~Source() = default;
        /** Connect to the source */
        virtual SourceError connect() = 0;
        /** Disconnect from the source */
        virtual SourceError disconnect() = 0;
        /** Send a command to the source */
        virtual SourceError sendCommand(
            const JkBmsDataBuffer command,
            const UuidStr service_uuid,
            const UuidStr char_uuid
        ) = 0;
        /** Subscribe to a characteristic */
        virtual SourceError subscribe(
            const UuidStr service_uuid,
            const UuidStr char_uuid,
            void* context,
            void(*callback)(void* context, const JkBmsDataBuffer data)
        ) = 0;
        /** Unsubscribe from a characteristic */
        virtual SourceError unsubscribe(
            const UuidStr service_uuid,
            const UuidStr char_uuid
        ) = 0;
        /** Get the MTU size of the source */
        virtual size_t getMtu() = 0;
    };

    class Controller
    {
    public:
        static UuidStr SERVICE_UUID;
        static UuidStr CHARACTERISTIC_UUID;
        constexpr static size_t MAX_PACKET_SIZE = 320;
        constexpr static size_t DEFAULT_PACKET_SIZE = 300;
    private:
        Source* source;
        Log& log;
        // TODO use JkBmsBuffer
        JkBmsByteBuffer responseBuffer;
        std::unique_ptr<DeviceInfoPromise> pendingDeviceInfoRequest;
        std::unique_ptr<CellInfoPromise> pendingCellInfoRequest;
        std::atomic<bool> cellInfoUpdatesRequested;
#ifdef JKBMSCPP_USE_STATEFLOW
        std::shared_ptr<flow::StateFlowImpl<CellInfo>> cellInfoStateFlow;
#endif        

        static void notificationCallback(
            void* ctx,
           const JkBmsDataBuffer data);
        void handleResponse(const JkBmsDataBuffer data) noexcept;
        // Ask BMS to start cell info periodic updates
        ControllerError requestCellInfoPeriodicUpdates() noexcept;
        void handleDeviceInfoResponse(const JkBmsDataBuffer data) noexcept;
        void handleCellInfoResponse(const JkBmsDataBuffer data) noexcept;
    public:
        Controller(Log& logger = (Log&)dumpLogInstance);
        virtual void start(Source* source);
        virtual void end();
        ~Controller();
        virtual DeviceInfoFuture readDeviceState();
        virtual CellInfoFuture readCellsState();
#ifdef JKBMSCPP_USE_STATEFLOW
        virtual std::shared_ptr<flow::StateFlow<CellInfo>> observeCellInfoState();
#endif
    private:
        static DumpLog dumpLogInstance;

};

} // namespace JkBmsCpp