#include <cstdint>
#pragma once
#ifdef JKBMSCPP_USE_STD_STRING
#include <string>
#endif
#include "CppWrappers.h"

namespace JkBmsCpp {
#ifdef JKBMSCPP_USE_STD_STRING    
    typedef std::string JkBmsString;
#else
    typedef const char* JkBmsString;
#endif

    enum class JkBmsSourceError: uint8_t {
        SUCCESS = 0,
    };

    enum class JkBmsControllerError: uint8_t {
        SUCCESS = 0,
        ERROR_NO_SOURCE = 1,
    };

    class JkBmsSource {
    public:
        virtual ~JkBmsSource() = default;
        virtual JkBmsSourceError connect() = 0;
        virtual JkBmsSourceError disconnect() = 0;
        virtual JkBmsSourceError sendCommand(
            const uint8_t* data, 
            const uint16_t size,
            const JkBmsString& service_uuid,
            const JkBmsString& char_uuid
        ) = 0;
        virtual JkBmsSourceError subscribe(
            const JkBmsString& service_uuid,
            const JkBmsString& char_uuid,
            void* context,
            void(*callback)(void* context, const uint8_t* data, const uint16_t size)
        ) = 0;
        virtual JkBmsSourceError unsubscribe(
            const JkBmsString& service_uuid,
            const JkBmsString& char_uuid
        ) = 0;
        virtual size_t getMtu() = 0;
    };

    class JkBmsDeviceInfoResponse {
        public:
            JkBmsString vendorId;
            JkBmsString hwVersion;
            JkBmsString swVersion;
            uint32_t uptimeSeconds;
            uint32_t powerOnCounter;
            JkBmsString deviceName;
            JkBmsString devicePasscode;
            JkBmsString manufacturedate;
            JkBmsString serialNumber;
            JkBmsString passcode;
            JkBmsString userData;
            JkBmsString setupPasscode;
    };

    class JkBmsController
    {
    public:
        const static JkBmsString SERVICE_UUID;
        const static JkBmsString CHARACTERISTIC_UUID;
    private:
        JkBmsSource* source;
        static void notificationCallback(
            void* ctx,
            const uint8_t* data,
            const uint16_t size);
        void handleResponse(const uint8_t* data, const uint16_t size);
        uint8_t calculateChecksum(const uint8_t* data, const uint16_t size);
    public:
        JkBmsController();
        virtual void start(JkBmsSource* source);
        virtual void end();
        ~JkBmsController();
        virtual Expected<JkBmsDeviceInfoResponse, JkBmsControllerError> readDeviceState();
    };

};