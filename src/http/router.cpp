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

#include "router.hpp"

using dasa::gliese::scanner::http::Router;
using namespace boost::beast::http;

response<dynamic_body> Router::handle_request(request<string_body>&& request) {
	auto handler = handlers.find(request.target());
	if (handler == handlers.end()) {
		return std::move(makeNotFoundResponse(request));
	}

	return std::move((*handler->second)(std::move(request)));
}

response<dynamic_body> Router::makeNotFoundResponse(const request<string_body>& request) {
	response<dynamic_body> res{ status::not_found, request.version() };
	res.set(field::server, BOOST_BEAST_VERSION_STRING);
	res.set(field::content_type, "text/plain");
	res.keep_alive(request.keep_alive());
	boost::beast::ostream(res.body()) << "not found";
	res.prepare_payload();
	return res;
}
