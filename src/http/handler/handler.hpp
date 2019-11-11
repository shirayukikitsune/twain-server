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

#include <boost/beast.hpp>
#include <kitsune/ioc/service>
#include <string>

boost::beast::http::response<boost::beast::http::dynamic_body> makeErrorResponse(boost::beast::http::status status, const std::string& message, const boost::beast::http::request<boost::beast::http::string_body> &request);

namespace dasa::gliese::scanner::http::handler {
    class RouteHandler : public kitsune::ioc::ServiceBase<RouteHandler> {
	public:
	    virtual ~RouteHandler() = default;
		[[nodiscard]] virtual boost::beast::http::verb method() const = 0;
		[[nodiscard]] virtual boost::beast::string_view route() const = 0;

        virtual boost::beast::http::response<boost::beast::http::dynamic_body> operator()(boost::beast::http::request<boost::beast::http::string_body> && request) {
            return makeErrorResponse(boost::beast::http::status::not_implemented, "not implemented", request);
        }
        virtual void operator()(boost::beast::http::request<boost::beast::http::string_body> && request, std::function<void(boost::beast::http::response<boost::beast::http::dynamic_body>)> send_fn) {
            send_fn((*this)(std::move(request)));
        }
	};

    template <boost::beast::http::verb Method>
    class RequestMapping : public RouteHandler {
        [[nodiscard]] boost::beast::http::verb method() const final {
            return Method;
        }
    };

    typedef RequestMapping<boost::beast::http::verb::get> GetMapping;
    typedef RequestMapping<boost::beast::http::verb::post> PostMapping;
    typedef RequestMapping<boost::beast::http::verb::put> PutMapping;
    typedef RequestMapping<boost::beast::http::verb::delete_> DeleteMapping;
    typedef RequestMapping<boost::beast::http::verb::options> OptionsMapping;
}
