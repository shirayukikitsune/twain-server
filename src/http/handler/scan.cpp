#include "handler.hpp"
#include "scan.hpp"
#include "handlers.hpp"
#include "../../application.hpp"
#include <chrono>
#include <loguru.hpp>
#include <streambuf>
#include <thread>

extern dasa::gliese::scanner::Application *application;

using dasa::gliese::scanner::http::handler::ScanHandler;
namespace bh = boost::beast::http;

static bh::response<bh::dynamic_body> makeErrorResponse(bh::status status, std::string message, const bh::request<bh::string_body> &request) {
    bh::response<bh::dynamic_body> res{ status, request.version() };
    res.set(bh::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(bh::field::content_type, "text/plain");
    res.keep_alive(request.keep_alive());
    boost::beast::ostream(res.body()) << message;
    res.prepare_payload();
    return res;
}

bh::response<bh::dynamic_body> ScanHandler::operator()(bh::request<bh::string_body>&& request) {
    auto rawBody = request.body();
    if (rawBody.empty()) {
        return makeErrorResponse(bh::status::unprocessable_entity, "Invalid body", request);
    }
    auto &twain = application->getTwain();
    nlohmann::json body;
    try {
        body = nlohmann::json::parse(request.body());
    }
    catch (std::exception &e) {
        LOG_S(WARNING) << "Failed to parse JSON body: " << e.what();
        return makeErrorResponse(bh::status::unprocessable_entity, "Invalid JSON body", request);
    }
    auto deviceId = body["device"];
    if (!deviceId.is_number_integer()) {
        return makeErrorResponse(bh::status::bad_request, "Device was not specified", request);
    }

    if (!twain.loadDataSource((TW_UINT32)deviceId)) {
        return makeErrorResponse(bh::status::internal_server_error, "Failed to load DS", request);
    }
        
    if (!twain.enableDataSource(GetDesktopWindow(), false)) {
        return makeErrorResponse(bh::status::internal_server_error, "Failed to enable DS", request);
    }

    // wait for ready
    while (twain.getState() != 6) {
        MSG msg = { };

        if (!GetMessage(&msg, nullptr, 0, 0)) {
            break;
        }
        // std::this_thread::sleep_for(std::chrono::milliseconds(10));

        TW_EVENT twEvent;

        twEvent.pEvent = (TW_MEMREF)&msg;
        twEvent.TWMessage = MSG_NULL;
        TW_UINT16  twRC = TWRC_NOTDSEVENT;
        twRC = twain.entry(twain.getIdentity(), twain.getDataSouce(), DG_CONTROL, DAT_EVENT, MSG_PROCESSEVENT, &twEvent);

        if (!twain.isUsingCallbacks() && twRC == TWRC_DSEVENT) {
            // check for message from Source
            switch (twEvent.TWMessage)
            {
            case MSG_XFERREADY:
                twain.setState(6);
            case MSG_CLOSEDSREQ:
            case MSG_CLOSEDSOK:
            case MSG_NULL:
                LOG_S(INFO) << "Got message from DSM: " << twEvent.TWMessage;
                break;
            default:
                LOG_S(INFO) << "Got unknown message from DSM: " << twEvent.TWMessage;
                break;
            }

            if (twEvent.TWMessage != MSG_NULL) {
                break;
            }
        }
        if (twRC != TWRC_DSEVENT) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    bh::response<bh::dynamic_body> response{ bh::status::ok, request.version() };
    response.set(bh::field::server, BOOST_BEAST_VERSION_STRING);

    if (twain.getState() == 6) {
        response.set(bh::field::content_type, "image/bmp");
        auto& os = boost::beast::ostream(response.body());
        twain.startScan(os);
    }

    twain.closeDS();

    response.prepare_payload();
    return response;
}
