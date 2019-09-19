#pragma once

#include <exception>
#include <sstream>

namespace dasa::gliese::scanner::exception {
    class HTTPException : public std::exception {};
    /*
    class HTTPBindException : public HTTPException {
    public:
        HTTPBindException(const char *addr, int port, int resultCode) : address(addr), port(port), resultCode(resultCode) {}

        [[nodiscard]] const char * what() const noexcept override {
            std::stringstream ss;
            ss << "Failed to bind to " << address << ":" << port;

            return ss.str().c_str();
        }

    private:
        const char *address;
        int port;
        int resultCode;
    };

    class HTTPListenException : public HTTPException {
    public:
        explicit HTTPListenException(int resultCode) : resultCode(resultCode) {}

        [[nodiscard]] const char * what() const noexcept override {
            std::stringstream ss;
            ss << "Failed to listen to connections: " << uv_strerror(resultCode);

            return ss.str().c_str();
        }

    private:
        int resultCode;
    };*/
}
