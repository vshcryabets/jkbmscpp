#pragma once

namespace JkBmsCpp {

    enum JkBmsSourceError {
        SUCCESS = 0,
    };

    class JkBmsSource {
    public:
        virtual ~JkBmsSource() = default;
        virtual JkBmsSourceError connect() = 0;
        virtual JkBmsSourceError disconnect() = 0;
    };

    class JkBmsController
    {
    private:
        JkBmsSource* source;
    public:
        JkBmsController();
        void start(JkBmsSource* source);
        void end();
        ~JkBmsController();
    };

};