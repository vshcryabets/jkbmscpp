#pragma once

#include <cstdint>
#include <future>
#include <memory>

#include "JkBmsFrames.h"
#include "CppWrappers.h"

namespace JkBmsCpp {

    typedef std::future<Expected<JkBmsDeviceInfoResponse, JkBmsControllerError>> DeviceInfoFuture;
    typedef std::promise<Expected<JkBmsDeviceInfoResponse, JkBmsControllerError>> DeviceInfoPromise;

    class JkBmsSource {
    public:
        virtual ~JkBmsSource() = default;
        virtual JkBmsSourceError connect() = 0;
        virtual JkBmsSourceError disconnect() = 0;
        virtual JkBmsSourceError sendCommand(
            const JkBmsDataBuffer& command,
            const JkBmsString& service_uuid,
            const JkBmsString& char_uuid
        ) = 0;
        virtual JkBmsSourceError subscribe(
            const JkBmsString& service_uuid,
            const JkBmsString& char_uuid,
            void* context,
            void(*callback)(void* context, const JkBmsDataBuffer &data)
        ) = 0;
        virtual JkBmsSourceError unsubscribe(
            const JkBmsString& service_uuid,
            const JkBmsString& char_uuid
        ) = 0;
        virtual size_t getMtu() = 0;
    };

    class JkBmsController
    {
    public:
        const static JkBmsString SERVICE_UUID;
        const static JkBmsString CHARACTERISTIC_UUID;
        constexpr static size_t MAX_PACKET_SIZE = 320;
        constexpr static size_t DEFAULT_PACKET_SIZE = 300;
    private:
        JkBmsSource* source;
        JkBmsByteBuffer responseBuffer;
        std::unique_ptr<DeviceInfoPromise> pendingDeviceInfoRequest;

        static void notificationCallback(
            void* ctx,
           const JkBmsDataBuffer &data);
        void handleResponse(const JkBmsDataBuffer &data);
        uint8_t calculateChecksum(const JkBmsDataBuffer &data);
    public:
        JkBmsController();
        virtual void start(JkBmsSource* source);
        virtual void end();
        ~JkBmsController();
        virtual DeviceInfoFuture readDeviceState();
    };

};