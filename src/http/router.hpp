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

#include "handler/handler.hpp"

#include <boost/beast.hpp>
#include <string>

namespace dasa::gliese::scanner::http {
	class Router {
	public:
		explicit Router(boost::beast::http::verb method) : method(std::move(method)) {}

		void add_handler(std::shared_ptr<handler::RouteHandler>&& handler) {
			handlers[handler->route()] = std::move(handler);
		}

		boost::beast::http::response<boost::beast::http::dynamic_body> handle_request(boost::beast::http::request<boost::beast::http::string_body>&& request);

	private:
		static boost::beast::http::response<boost::beast::http::dynamic_body> makeNotFoundResponse(const boost::beast::http::request<boost::beast::http::string_body>& request);

		std::map<boost::beast::string_view, std::shared_ptr<handler::RouteHandler>> handlers;
		boost::beast::http::verb method;
	};
}
