#include "scan.hpp"

#include "../../application.hpp"

#include <chrono>
#include <loguru.hpp>
#include <thread>

KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::PrepareScanHandler, prepareScanHandlerInjectable);
KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::NextImageDataScanHandler, nextImageDataScanHandlerInjectable);
KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::HasNextScanHandler, hasNextScanHandlerInjectable);
KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::NextScanHandler, nextScanHandlerInjectable);
KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::EndScanHandler, endScanHandlerInjectable);
KITSUNE_INJECTABLE(dasa::gliese::scanner::http::handler::RouteHandler, dasa::gliese::scanner::http::handler::ScanHandler, scanHandlerInjectable);

extern dasa::gliese::scanner::Application *application;

using namespace dasa::gliese::scanner::http::handler;
namespace bh = boost::beast::http;

static bh::response<bh::dynamic_body> makeErrorResponse(bh::status status, const std::string& message, const bh::request<bh::string_body> &request) {
    bh::response<bh::dynamic_body> res{ status, request.version() };
    res.set(bh::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(bh::field::content_type, "text/plain");
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
		twain.setCapability(ICAP_XRESOLUTION, &res);
	}

	if (!twain.enableDataSource(application->getParentWindow(), false)) {
		return makeErrorResponse(bh::status::internal_server_error, "Failed to enable DS", request);
	}

	// wait for ready
	while (twain.getState() != 6) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	bh::response<bh::dynamic_body> response{ bh::status::ok, request.version() };
	response.set(bh::field::server, BOOST_BEAST_VERSION_STRING);

	return response;
}

std::unique_ptr<dasa::gliese::scanner::twain::Transfer> transfer;

bh::response<bh::dynamic_body> PrepareScanHandler::operator()(bh::request<bh::string_body>&& request) {
	auto response = prepareScan(request);
	auto& twain = application->getTwain();

	if (twain.getState() == 6) {
		transfer = twain.startScan();
	}

	response.prepare_payload();
	return response;
}

bh::response<bh::dynamic_body> NextImageDataScanHandler::operator()(bh::request<bh::string_body>&& request) {
	if (!transfer) {
		return makeErrorResponse(bh::status::precondition_failed, "Transfer not initiated", request);
	}

	bh::response<bh::dynamic_body> response{ bh::status::ok, request.version() };
	response.set(bh::field::server, BOOST_BEAST_VERSION_STRING);
	response.set(bh::field::content_type, "application/json");

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
	body["planar"] = imageInfo.Planar;
	body["samplesPerPixel"] = imageInfo.SamplesPerPixel;
	body["xResolution"] = imageInfo.XResolution.Whole;
	body["yResolution"] = imageInfo.YResolution.Whole;
	boost::beast::ostream(response.body()) << body.dump();
	response.prepare_payload();
	return response;
}

bh::response<bh::dynamic_body> HasNextScanHandler::operator()(bh::request<bh::string_body>&& request) {
	if (!transfer) {
		return makeErrorResponse(bh::status::precondition_failed, "Transfer not initiated", request);
	}

	bh::response<bh::dynamic_body> response{ bh::status::ok, request.version() };
	response.set(bh::field::server, BOOST_BEAST_VERSION_STRING);
	response.set(bh::field::content_type, "application/json");

	nlohmann::json body;
	body["hasNext"] = transfer->hasPending();

	boost::beast::ostream(response.body()) << body.dump();
	response.prepare_payload();
	return response;
}

bh::response<bh::dynamic_body> NextScanHandler::operator()(bh::request<bh::string_body>&& request) {
	if (!transfer) {
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
	response.set(bh::field::content_type, "image/bmp");
	auto os = boost::beast::ostream(response.body());

	transfer->transferOne(os);
	transfer->checkPending();

	response.set("x-has-next", transfer->hasPending());

	response.prepare_payload();
	return response;
}

bh::response<bh::dynamic_body> EndScanHandler::operator()(bh::request<bh::string_body>&& request) {
	if (!transfer) {
		return makeErrorResponse(bh::status::precondition_failed, "Transfer not initiated", request);
	}

	bh::response<bh::dynamic_body> response{ bh::status::no_content, request.version() };
	response.set(bh::field::server, BOOST_BEAST_VERSION_STRING);

	transfer->clearPending();
	transfer->end();

	auto& twain = application->getTwain();
	twain.closeDS();
	transfer = nullptr;

	response.prepare_payload();
	return response;
}

bh::response<bh::dynamic_body> ScanHandler::operator()(bh::request<bh::string_body>&& request) {
	auto response = prepareScan(request);
	auto& twain = application->getTwain();

    if (twain.getState() == 6) {
        response.set(bh::field::content_type, "image/bmp");
        auto os = boost::beast::ostream(response.body());
        transfer = twain.startScan();
		transfer->transferAll(os);
    }

    twain.closeDS();

    transfer = nullptr;

    response.prepare_payload();
    return response;
}
