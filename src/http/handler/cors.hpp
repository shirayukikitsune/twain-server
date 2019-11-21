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

#include "handler.hpp"

#include <kitsune/ioc/service>

namespace dasa::gliese::scanner::http::handler {

    void add_cors(const boost::beast::http::request<boost::beast::http::string_body>& request, boost::beast::http::response<boost::beast::http::dynamic_body>& response);

    template <typename DerivedType>
    class CORSMapping : public kitsune::ioc::Service<DerivedType, OptionsMapping> {
    public:
        boost::beast::http::response<boost::beast::http::dynamic_body> operator()(boost::beast::http::request<boost::beast::http::string_body> && request) final {
            namespace bh = boost::beast::http;
            bh::response<bh::dynamic_body> res{ bh::status::ok, request.version() };
            res.set(bh::field::server, BOOST_BEAST_VERSION_STRING);

            auto origin = request[bh::field::origin];
            if (!origin.empty()) {
                res.set(bh::field::access_control_allow_origin, origin);
                res.set(bh::field::access_control_allow_methods, methods());
                res.set(bh::field::access_control_allow_headers, headers());
            }

            res.keep_alive(request.keep_alive());
            res.prepare_payload();
            return res;
        }

    protected:
        [[nodiscard]] virtual boost::beast::string_view headers() {
            return "Server, Content-Type";
        }

        [[nodiscard]] virtual boost::beast::string_view methods() {
            return "GET";
        }
    };

}
