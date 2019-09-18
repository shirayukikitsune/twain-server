#pragma once

#include <exception>
#include <sstream>

namespace dasa::gliese::scanner::exception {

    class DSMException : public std::exception {
    };

    class DSMOpenException : public DSMException {
    public:
        explicit DSMOpenException(unsigned short resultCode) : resultCode(resultCode) {}

        [[nodiscard]] const char * what() const noexcept override {
            std::stringstream ss;
            ss << "Failed to open DSM connection [RC: " << resultCode << "]";
            return ss.str().c_str();
        }

    private:
        unsigned short resultCode;
    };

}
