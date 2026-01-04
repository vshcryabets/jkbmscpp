#pragma once
#ifdef JKBMSCPP_USE_STD_STRING
#include <string>
#endif

namespace JkBmsCpp {
#ifdef JKBMSCPP_USE_STD_STRING    
    typedef std::string JkBmsString;
#endif

    enum JkBmsSourceError {
        SUCCESS = 0,
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
            const void* context,
            void(*callback)(const void* context, const uint8_t* data, const uint16_t size)
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
    private:
        JkBmsSource* source;
        static void notificationCallback(
            const void* ctx,
            const uint8_t* data,
            const uint16_t size);
        void handleResponse(const uint8_t* data, const uint16_t size);
    public:
        JkBmsController();
        virtual void start(JkBmsSource* source);
        virtual void end();
        ~JkBmsController();
        virtual void readDeviceState();
    };

};