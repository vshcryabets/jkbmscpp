#pragma once

#include <cstdint>
#include <future>
#include <memory>

#include "JkBmsFrames.h"
#include "CppWrappers.h"

namespace JkBmsCpp {

    typedef std::future<Expected<DeviceInfoResponse, ControllerError>> DeviceInfoFuture;
    typedef std::promise<Expected<DeviceInfoResponse, ControllerError>> DeviceInfoPromise;

    typedef std::future<Expected<CellInfoResponse, ControllerError>> CellInfoFuture;
    typedef std::promise<Expected<CellInfoResponse, ControllerError>> CellInfoPromise;

    class Source {
    public:
        virtual ~Source() = default;
        /** Connect to the source */
        virtual SourceError connect() = 0;
        /** Disconnect from the source */
        virtual SourceError disconnect() = 0;
        /** Send a command to the source */
        virtual SourceError sendCommand(
            const JkBmsDataBuffer& command,
            const JkBmsString& service_uuid,
            const JkBmsString& char_uuid
        ) = 0;
        /** Subscribe to a characteristic */
        virtual SourceError subscribe(
            const JkBmsString& service_uuid,
            const JkBmsString& char_uuid,
            void* context,
            void(*callback)(void* context, const JkBmsDataBuffer &data)
        ) = 0;
        /** Unsubscribe from a characteristic */
        virtual SourceError unsubscribe(
            const JkBmsString& service_uuid,
            const JkBmsString& char_uuid
        ) = 0;
        /** Get the MTU size of the source */
        virtual size_t getMtu() = 0;
    };

    class Controller
    {
    public:
        const static JkBmsString SERVICE_UUID;
        const static JkBmsString CHARACTERISTIC_UUID;
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
           const JkBmsDataBuffer &data);
        void handleResponse(const JkBmsDataBuffer &data);
    public:
        Controller();
        virtual void start(Source* source);
        virtual void end();
        ~Controller();
        virtual DeviceInfoFuture readDeviceState();
        virtual CellInfoFuture readCellsState();
    };

};