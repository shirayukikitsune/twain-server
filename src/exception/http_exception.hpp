/*
    twain-server: This project exposes the TWAIN DSM as a web API
    Copyright (C) 2019 "Diagnósticos da América S.A. <bruno.f@dasa.com.br>"

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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
