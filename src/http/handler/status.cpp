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

#include "status.hpp"
#include "../../application.hpp"

KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::StatusHandler, statusHandlerInjectable);
KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::ResetHandler, resetHandlerInjectable);

extern dasa::gliese::scanner::Application *application;

using namespace dasa::gliese::scanner::http::handler;
namespace bh = boost::beast::http;

bh::response<bh::dynamic_body> StatusHandler::operator()(bh::request<bh::string_body>&& request) {
    nlohmann::json response;
    response["status"] = "UP";
	response["details"] = {
		{"twain", application->getTwain().getState()}
	};

    bh::response<bh::dynamic_body> res{ bh::status::ok, request.version() };
	res.set(bh::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(bh::field::content_type, "application/json");
	res.keep_alive(request.keep_alive());
	boost::beast::ostream(res.body()) << response;
	res.prepare_payload();
	return res;
}

bh::response<bh::dynamic_body> ResetHandler::operator()(bh::request<bh::string_body> &&request) {
    application->getTwain().reset();

    StatusHandler statusHandler;
    return statusHandler(std::move(request));
}
