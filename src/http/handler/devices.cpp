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

#include "devices.hpp"
#include "../../application.hpp"
#include <cstdlib>

KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::DevicesHandler, devicesHandlerInjectable);
KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::DevicesCORSHandler, devicesCorsHandlerInjectable);
KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::DevicesDPIHandler, devicesDPIHandlerInjectable);
KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::DevicesDPICORSHandler, devicesDPICorsHandlerInjectable);

extern dasa::gliese::scanner::Application *application;

using namespace dasa::gliese::scanner::http::handler;
using nlohmann::json;
namespace bh = boost::beast::http;

bh::response<bh::dynamic_body> DevicesHandler::operator()(bh::request<bh::string_body>&& request) {
	json response;
    auto devices = application->getTwain().listSources();
    auto defaultDevice = application->getTwain().getDefaultDataSource();
    size_t i = 0;
    for (auto & device : devices) {
        auto deviceJson = device.toJson();
        if (device == defaultDevice.Id) {
            deviceJson["default"] = true;
        }
        response[i++] = deviceJson;
    }

    bh::response<bh::dynamic_body> res{ bh::status::ok, request.version() };
	res.set(bh::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(bh::field::content_type, "application/json");

	auto origin = request[bh::field::origin];
	if (!origin.empty()) {
        res.set(bh::field::access_control_allow_origin, origin);
    }
	res.keep_alive(request.keep_alive());
	boost::beast::ostream(res.body()) << response;
	res.prepare_payload();
	return res;
}

bh::response<bh::dynamic_body> DevicesCORSHandler::operator()(bh::request<bh::string_body>&& request) {
    bh::response<bh::dynamic_body> res{ bh::status::ok, request.version() };
    res.set(bh::field::server, BOOST_BEAST_VERSION_STRING);

    auto origin = request[bh::field::origin];
    if (!origin.empty()) {
        res.set(bh::field::access_control_allow_origin, origin);
        res.set(bh::field::access_control_allow_methods, "GET");
        res.set(bh::field::access_control_allow_headers, "Server, Content-Type");
    }

    res.keep_alive(request.keep_alive());
    res.prepare_payload();
    return res;
}

bh::response<bh::dynamic_body> DevicesDPIHandler::operator()(bh::request<bh::string_body>&& request) {
	json response;
	auto device = atoll(((std::string)request["x-device"]).c_str());
	TW_CAPABILITY cap{ICAP_XRESOLUTION, 0, nullptr};
	application->getTwain().loadDataSource(device);
    application->getTwain().getCapability(cap);

    if (cap.ConType == TWON_ENUMERATION) {
        auto con = ((TW_ENUMERATION*)cap.hContainer);
        for (unsigned i = 0; i < con->NumItems; ++i) {
            auto res = reinterpret_cast<pTW_FIX32>(&con->ItemList)[i];
            response[i] = res.Whole;
        }
    }

    application->getTwain().dsm().free(cap.hContainer);
    application->getTwain().closeDS();

    bh::response<bh::dynamic_body> res{ bh::status::ok, request.version() };
	res.set(bh::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(bh::field::content_type, "application/json");

	auto origin = request[bh::field::origin];
	if (!origin.empty()) {
        res.set(bh::field::access_control_allow_origin, origin);
    }
	res.keep_alive(request.keep_alive());
	boost::beast::ostream(res.body()) << response;
	res.prepare_payload();
	return res;
}

bh::response<bh::dynamic_body> DevicesDPICORSHandler::operator()(bh::request<bh::string_body>&& request) {
    bh::response<bh::dynamic_body> res{ bh::status::ok, request.version() };
    res.set(bh::field::server, BOOST_BEAST_VERSION_STRING);

    auto origin = request[bh::field::origin];
    if (!origin.empty()) {
        res.set(bh::field::access_control_allow_origin, origin);
        res.set(bh::field::access_control_allow_methods, "GET");
        res.set(bh::field::access_control_allow_headers, "Server, Content-Type, X-Device");
    }

    res.keep_alive(request.keep_alive());
    res.prepare_payload();
    return res;
}
