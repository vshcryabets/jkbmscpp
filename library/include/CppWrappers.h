#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

#ifdef JKBMSCPP_USE_STD_STRING
#include <string>
#endif

namespace JkBmsCpp {
    template<typename T, typename E>
    class Expected {
        private:
            union {
                T _value;
                E _error;
            };
            bool _has_value;
        public:
            Expected(const T& val) : _value(val), _has_value(true) {}
            Expected(const E& err) : _error(err), _has_value(false) {}
            Expected(const Expected& other) : _has_value(other._has_value) {
                if (_has_value) {
                    new(&_value) T(other._value);
                } else {
                    new(&_error) E(other._error);
                }
            }
            bool hasValue() const { return _has_value; }
            const T& value() const { return _value; }
            const E& error() const { return _error; }
            ~Expected() {
                if (_has_value) {
                    _value.~T();
                } else {
                    _error.~E();
                }
            }
    };

#ifdef JKBMSCPP_USE_STD_STRING    
    typedef std::string JkBmsString;
#else
    typedef const char* JkBmsString;
#endif

    typedef std::vector<uint8_t> JkBmsByteBuffer;

    class JkBmsDataBuffer {
        private:
            uint8_t* _data;
            uint16_t _size;
        public:
            JkBmsDataBuffer(uint8_t* data, uint16_t size)
                : _data(data), _size(size) {}
            ~JkBmsDataBuffer() = default;
            uint8_t* data() const { return _data; }
            uint16_t size() const { return _size; }            
    };
}