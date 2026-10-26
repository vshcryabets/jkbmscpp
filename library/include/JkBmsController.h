#pragma once

#include <cstdint>
#include <future>
#include <memory>

#include "JkBmsFrames.h"
#include "CppWrappers.h"
#include "CFunctions.h"

namespace JkBmsCpp {

    typedef std::future<Expected<DeviceInfoResponse, ControllerError_C>> DeviceInfoFuture;
    typedef std::promise<Expected<DeviceInfoResponse, ControllerError_C>> DeviceInfoPromise;

    typedef std::future<Expected<CellInfoResponse, ControllerError_C>> CellInfoFuture;
    typedef std::promise<Expected<CellInfoResponse, ControllerError_C>> CellInfoPromise;

    class Source {
    public:
        virtual ~Source() = default;
        /** Connect to the source */
        virtual SourceError connect() = 0;
        /** Disconnect from the source */
        virtual SourceError disconnect() = 0;
        /** Send a command to the source */
        virtual SourceError sendCommand(
            const JkBmsDataBuffer_C command,
            const UuidStr service_uuid,
            const UuidStr char_uuid
        ) = 0;
        /** Subscribe to a characteristic */
        virtual SourceError subscribe(
            const UuidStr service_uuid,
            const UuidStr char_uuid,
            void* context,
            void(*callback)(void* context, const JkBmsDataBuffer_C data)
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
        // TODO use JkBmsBuffer
        JkBmsByteBuffer responseBuffer;
        std::unique_ptr<DeviceInfoPromise> pendingDeviceInfoRequest;
        std::unique_ptr<CellInfoPromise> pendingCellInfoRequest;

        static void notificationCallback(
            void* ctx,
           const JkBmsDataBuffer_C data);
        void handleResponse(const JkBmsDataBuffer_C data);
    public:
        Controller();
        virtual void start(Source* source);
        virtual void end();
        ~Controller();
        virtual DeviceInfoFuture readDeviceState();
        virtual CellInfoFuture readCellsState();
    };

};