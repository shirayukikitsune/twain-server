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
#include <boost/asio/yield.hpp>
#include <loguru.hpp>

KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::DevicesHandler, devicesHandlerInjectable)
KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::DevicesCORSHandler, devicesCorsHandlerInjectable)
KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::DevicesAsyncHandler, devicesAsyncHandlerInjectable)
KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::DevicesAsyncCORSHandler, devicesAsyncCorsHandlerInjectable)
KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::DevicesDPIHandler, devicesDPIHandlerInjectable)
KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::DevicesDPICORSHandler, devicesDPICorsHandlerInjectable)

extern dasa::gliese::scanner::Application *application;

using namespace dasa::gliese::scanner::http::handler;
using nlohmann::json;
namespace bh = boost::beast::http;

void DevicesAsyncHandler::handle(const bh::request<bh::string_body>& request, std::function<void(boost::beast::http::response<boost::beast::http::dynamic_body>)> send, boost::asio::coroutine co, std::error_code ec, std::list<dasa::gliese::scanner::twain::Device> devices) {
    reenter(co) {
        for (;;) {
            yield application->getTwain().async_list_sources(std::bind(&DevicesAsyncHandler::handle, this, request, send, co, std::placeholders::_1, std::placeholders::_2));

            break;
        }

        if (ec) {
            send(makeErrorResponse(bh::status::service_unavailable, "TWAIN DSM connection is unavailable", request));
            return;
        }

        json response_body;
        auto defaultDevice = application->getTwain().getDefaultDataSource();
        size_t i = 0;
        for (auto& device : devices) {
            auto deviceJson = device.toJson();
            if (device == defaultDevice.Id) {
                deviceJson["default"] = true;
            }
            response_body[i++] = deviceJson;
        }

        bh::response<bh::dynamic_body> res{ bh::status::ok, request.version() };
        res.set(bh::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(bh::field::content_type, "application/json");

        auto origin = request[bh::field::origin];
        if (!origin.empty()) {
            res.set(bh::field::access_control_allow_origin, origin);
        }
        res.keep_alive(request.keep_alive());
        boost::beast::ostream(res.body()) << response_body;
        res.prepare_payload();
        send(res);
    }

}

void DevicesAsyncHandler::operator()(boost::beast::http::request<boost::beast::http::string_body>&& request, std::function<void(boost::beast::http::response<boost::beast::http::dynamic_body>)> send) {
    boost::asio::coroutine co;
    LOG_S(INFO) << "Requesting async device list";
    handle(request, send, co, {}, {});
}

bh::response<bh::dynamic_body> DevicesHandler::operator()(bh::request<bh::string_body>&& request) {
    LOG_SCOPE_F(INFO, "GET /devices");
	json response;
	std::error_code ec;
    auto devices = application->getTwain().listSources(ec);
    if (ec) {
        return makeErrorResponse(bh::status::service_unavailable, "TWAIN DSM connection is unavailable", request);
    }

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

bh::response<bh::dynamic_body> DevicesDPIHandler::operator()(bh::request<bh::string_body>&& request) {
    LOG_SCOPE_F(INFO, "GET /devices/dpi");
	json response;
	auto device = (twain::Device::TW_ID)strtoll(((std::string)request["x-device"]).c_str(), nullptr, 0);
	if (!device) {
        return makeErrorResponse(bh::status::not_found, "device not found", request);
	}

    auto dpis = application->getTwain().getDeviceDPIs(device);
    response["x"] = std::get<0>(dpis);
    response["y"] = std::get<1>(dpis);

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
