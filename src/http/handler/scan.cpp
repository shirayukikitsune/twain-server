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

#include "scan.hpp"

#include "../../application.hpp"
#include "../../twain/error_code.hpp"

#include <chrono>
#include <loguru.hpp>
#include <thread>

KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::PrepareScanHandler, prepareScanHandlerInjectable)
KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::NextImageDataScanHandler, nextImageDataScanHandlerInjectable)
KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::HasNextScanHandler, hasNextScanHandlerInjectable)
KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::NextScanHandler, nextScanHandlerInjectable)
KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::EndScanHandler, endScanHandlerInjectable)
KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::ScanHandler, scanHandlerInjectable)
KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::PrepareScanCORSHandler, prepareScanCorsHandlerInjectable)
KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::NextImageDataScanCORSHandler, nextImageDataScanCorsHandlerInjectable)
KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::HasNextScanCORSHandler, hasNextScanCorsHandlerInjectable)
KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::NextScanCORSHandler, nextScanCorsHandlerInjectable)
KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::EndScanCORSHandler, endScanCorsHandlerInjectable)
KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::ScanCORSHandler, scanCorsHandlerInjectable)

extern dasa::gliese::scanner::Application *application;

using namespace dasa::gliese::scanner::http::handler;
namespace bh = boost::beast::http;

bh::response<bh::dynamic_body> makeErrorResponse(bh::status status, const std::string& message, const bh::request<bh::string_body> &request) {
    bh::response<bh::dynamic_body> res{ status, request.version() };
    res.set(bh::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(bh::field::content_type, "text/plain");
    add_cors(request, res);
    res.keep_alive(request.keep_alive());
    boost::beast::ostream(res.body()) << message;
    res.prepare_payload();
    return res;
}

bh::response<bh::dynamic_body> prepareScan(const bh::request<bh::string_body>& request) {
	const auto& rawBody = request.body();
	if (rawBody.empty()) {
		return makeErrorResponse(bh::status::unprocessable_entity, "Invalid body", request);
	}
	auto& twain = application->getTwain();
	nlohmann::json body;
	try {
		body = nlohmann::json::parse(request.body());
	}
	catch (std::exception & e) {
		LOG_S(WARNING) << "Failed to parse JSON body: " << e.what();
		return makeErrorResponse(bh::status::unprocessable_entity, "Invalid JSON body", request);
	}
	auto deviceId = body["device"];
	if (!deviceId.is_number_integer()) {
		return makeErrorResponse(bh::status::bad_request, "Device was not specified", request);
	}

#ifdef __APPLE__
	auto id = (unsigned long)deviceId;
	if (!twain.loadDataSource(reinterpret_cast<TW_MEMREF>(id))) {
#else
	if (!twain.loadDataSource((TW_UINT32)deviceId)) {
#endif
        twain.reset();
		return makeErrorResponse(bh::status::internal_server_error, "Failed to load DS", request);
	}

	auto pixelType = body["pixelType"];
	if (pixelType.is_string()) {
		TW_UINT16 twPixelType = TWPT_BW;
		std::string sPixelType = (std::string)pixelType;
		if (sPixelType == "rgb") {
			twPixelType = TWPT_RGB;
		}
		else if (sPixelType == "gray") {
			twPixelType = TWPT_GRAY;
		}

		twain.setCapability(ICAP_PIXELTYPE, twPixelType, TWTY_UINT16);
	}

	auto xResolution = body["resolution"]["x"];
	if (xResolution.is_number_integer()) {
		TW_FIX32 res;
		res.Frac = 0;
		res.Whole = (TW_INT16)body["resolution"]["x"];
		twain.setCapability(ICAP_XRESOLUTION, &res);
	}

	auto yResolution = body["resolution"]["y"];
	if (xResolution.is_number_integer()) {
		TW_FIX32 res;
		res.Frac = 0;
		res.Whole = (TW_INT16)body["resolution"]["y"];
		twain.setCapability(ICAP_YRESOLUTION, &res);
	}

	auto xferMode = body["transferMode"];
	int twsx = TWSX_MEMORY;
	if (xferMode.is_string()) {
	    auto mode = (std::string)xferMode;
	    if (mode == "native") {
	        twsx = TWSX_NATIVE;
	    }
	}
    twain.setCapability(ICAP_XFERMECH, twsx, TWTY_UINT16);

	auto useDuplex = body["duplex"];
	if (useDuplex.is_boolean()) {
	    auto mode = (bool)useDuplex ? 1 : 0;
	    twain.setCapability(CAP_DUPLEXENABLED, mode, TWTY_BOOL);
	}

	// Force LSB transfer
	twain.setCapability(ICAP_BITORDER, TWBO_MSBFIRST, TWTY_UINT16);
	twain.setCapability(ICAP_BITORDERCODES, TWBO_MSBFIRST, TWTY_UINT16);

    twain.enableDataSource(application->getParentWindow(), false);

	// wait for ready
    using namespace std::literals;
	while (twain.getState() != 6) {
		std::this_thread::sleep_for(10ms);
	}

	bh::response<bh::dynamic_body> response{ bh::status::no_content, request.version() };
	response.set(bh::field::server, BOOST_BEAST_VERSION_STRING);
    add_cors(request, response);

	return response;
}

bh::response<bh::dynamic_body> PrepareScanHandler::operator()(bh::request<bh::string_body>&& request) {
    LOG_SCOPE_F(INFO, "POST /scan/prepare");
	auto response = prepareScan(request);
	auto& twain = application->getTwain();
	auto outputMime = (std::string)request[bh::field::accept];

	if (twain.getState() == 6) {
        auto transfer = twain.startScan(outputMime);
        if (transfer.expired()) {
            twain.reset();
            return makeErrorResponse(bh::status::internal_server_error, "Failed to initialize transfer", request);
        }
	}
    add_cors(request, response);

	response.prepare_payload();
	return response;
}

bh::response<bh::dynamic_body> HasNextScanHandler::operator()(bh::request<bh::string_body>&& request) {
    LOG_SCOPE_F(INFO, "GET /scan/has-next");
    auto transfer = application->getTwain().getActiveTransfer();
    if (!transfer) {
        application->getTwain().reset();
        return makeErrorResponse(bh::status::precondition_failed, "Transfer not initiated", request);
    }

    bh::response<bh::dynamic_body> response{ bh::status::ok, request.version() };
    response.set(bh::field::server, BOOST_BEAST_VERSION_STRING);
    response.set(bh::field::content_type, "application/json");
    add_cors(request, response);

    nlohmann::json body;
    body["hasNext"] = transfer->hasPending();

    boost::beast::ostream(response.body()) << body.dump();
    response.prepare_payload();
    return response;
}

bh::response<bh::dynamic_body> NextImageDataScanHandler::operator()(bh::request<bh::string_body>&& request) {
    LOG_SCOPE_F(INFO, "GET /scan/prepare-next");
    auto transfer = application->getTwain().getActiveTransfer();
    if (!transfer) {
        application->getTwain().reset();
        return makeErrorResponse(bh::status::precondition_failed, "Transfer not initiated", request);
	}

	bh::response<bh::dynamic_body> response{ bh::status::ok, request.version() };
	response.set(bh::field::server, BOOST_BEAST_VERSION_STRING);
	response.set(bh::field::content_type, "application/json");
    add_cors(request, response);

	auto imageInfo = transfer->prepare();
	nlohmann::json body;
	body["bitsPerPixel"] = imageInfo.BitsPerPixel;
	for (int i = 0; i < 8; ++i) {
		body["bitsPerSample"][i] = imageInfo.BitsPerSample[i];
	}
	body["compression"] = imageInfo.Compression;
	body["length"] = imageInfo.ImageLength;
	body["width"] = imageInfo.ImageWidth;
	body["pixelType"] = imageInfo.PixelType;
	body["planar"] = imageInfo.Planar != 0;
	body["samplesPerPixel"] = imageInfo.SamplesPerPixel;
	body["xResolution"] = imageInfo.XResolution.Whole;
	body["yResolution"] = imageInfo.YResolution.Whole;
	boost::beast::ostream(response.body()) << body.dump();
	response.prepare_payload();
	return response;
}

bh::response<bh::dynamic_body> NextScanHandler::operator()(bh::request<bh::string_body>&& request) {
    LOG_SCOPE_F(INFO, "GET /scan/next");
    auto transfer = application->getTwain().getActiveTransfer();
    auto& twain = application->getTwain();
    if (!transfer) {
        twain.reset();
        return makeErrorResponse(bh::status::precondition_failed, "Transfer not initiated", request);
	}
	if (!transfer->hasPending()) {
		bh::response<bh::dynamic_body> response{ bh::status::no_content, request.version() };
		response.set(bh::field::server, BOOST_BEAST_VERSION_STRING);
		response.prepare_payload();
		return response;
	}

	bh::response<bh::dynamic_body> response{ bh::status::ok, request.version() };
	response.set(bh::field::server, BOOST_BEAST_VERSION_STRING);
	response.set(bh::field::content_type, transfer->getTransferMIME());
    add_cors(request, response);

	auto os = boost::beast::ostream(response.body());

    std::error_code ec;
	transfer->transferOne(os, ec);
    if (ec) {
        transfer->clearPending();
        twain.endTransfer();

        twain.disableDS();
        twain.closeDS();

        if (ec == twain::error_code::cancelled) {
            return makeErrorResponse(bh::status::client_closed_request, "User cancelled", request);
        }
        return makeErrorResponse(bh::status::internal_server_error, "Failed to transfer", request);
    }
	transfer->checkPending();

	response.set("x-has-next", transfer->hasPending());

	response.prepare_payload();
	return response;
}

bh::response<bh::dynamic_body> EndScanHandler::operator()(bh::request<bh::string_body>&& request) {
    LOG_SCOPE_F(INFO, "POST /scan/end");
    auto transfer = application->getTwain().getActiveTransfer();
    if (!transfer) {
        auto& twain = application->getTwain();
        twain.reset();
        return makeErrorResponse(bh::status::precondition_failed, "Transfer not initiated", request);
	}

	bh::response<bh::dynamic_body> response{ bh::status::no_content, request.version() };
	response.set(bh::field::server, BOOST_BEAST_VERSION_STRING);
    add_cors(request, response);

    auto& twain = application->getTwain();
	transfer->clearPending();
    twain.endTransfer();

	twain.disableDS();
	twain.closeDS();

	response.prepare_payload();
	return response;
}

bh::response<bh::dynamic_body> ScanHandler::operator()(bh::request<bh::string_body>&& request) {
    LOG_SCOPE_F(INFO, "POST /scan");
    auto response = prepareScan(request);
	auto& twain = application->getTwain();

    if (twain.getState() == 6) {
        auto outputMime = (std::string)request[bh::field::accept];
        add_cors(request, response);
        auto os = boost::beast::ostream(response.body());
        auto transfer = twain.startScan(outputMime);
        if (transfer.expired()) {
            twain.reset();
            return makeErrorResponse(bh::status::internal_server_error, "Failed to start transfer", request);
        }
        response.set(bh::field::content_type, "image/jpeg");
		transfer.lock()->transferAll(os);
    }

    twain.endTransfer();
    twain.disableDS();
    twain.closeDS();

    response.prepare_payload();
    return response;
}
