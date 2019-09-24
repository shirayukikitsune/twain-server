#include "handlers.hpp"
#include "../../application.hpp"
#include <chrono>
#include <thread>

extern dasa::gliese::scanner::Application *application;

class ScanHandler : public dasa::gliese::scanner::http::RouteHandler {
public:
    [[nodiscard]] web::http::method method() const final {
        return web::http::methods::POST;
    }

    [[nodiscard]] utility::string_t route() const final {
        return U("/scan");
    }

    void operator()(const web::http::http_request& request) final {
        concurrency::streams::ostream os;
        auto &twain = application->getTwain();
        auto body = request.extract_json().get();
        auto deviceId = body[U("device")];
        if (!deviceId.is_integer()) {
            request.reply(web::http::status_codes::BadRequest, makeErrorResponse(U("Device was not specified")));
            return;
        }

        if (!twain.loadDataSource((TW_UINT32)deviceId.as_integer())) {
            request.reply(web::http::status_codes::InternalError, makeErrorResponse(U("Failed to load DS")));
            return;
        }

        if (!twain.enableDataSource(nullptr, false)) {
            request.reply(web::http::status_codes::InternalError, makeErrorResponse(U("Failed to enable DS")));
            return;
        }

        // wait for ready
        while (twain.getState() != 6) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        TW_SETUPMEMXFER sourceBufferSize;
        memset(&sourceBufferSize, 0, sizeof(TW_SETUPMEMXFER));
        twain.entry(twain.getIdentity(), twain.getDataSouce(), DG_CONTROL, DAT_SETUPMEMXFER, MSG_GET, &sourceBufferSize);

        concurrency::streams::producer_consumer_buffer<BYTE> buffer(sourceBufferSize.Preferred);
        request.reply(web::http::status_codes::OK, buffer.create_istream());
        auto bufferOs = buffer.create_ostream();
        twain.startScan(bufferOs);
        bufferOs.close();

        twain.closeDS();
    }

private:
    static web::json::value makeErrorResponse(utility::string_t message) {
        web::json::value responseObject = web::json::value::object();
        responseObject[U("error")] = web::json::value::string(std::move(message));

        return responseObject;
    }
};
