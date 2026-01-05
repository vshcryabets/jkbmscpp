#pragma once

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
}