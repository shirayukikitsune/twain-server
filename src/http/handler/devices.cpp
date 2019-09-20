#include "handlers.hpp"
#include "../../application.hpp"

extern dasa::gliese::scanner::Application *application;

class DevicesHandler : public dasa::gliese::scanner::http::RouteHandler {
    [[nodiscard]] web::http::method method() const final {
        return web::http::methods::GET;
    }

    [[nodiscard]] utility::string_t route() const final {
        return U("/devices");
    }

    void operator()(const web::http::http_request& request) final {
        auto response = web::json::value::array();
        auto devices = application->getTwain().listSources();
        auto defaultDevice = application->getTwain().getDefaultDataSource();
        size_t i = 0;
        for (auto & device : devices) {
            auto deviceJson = deviceToJson(device);
            if (defaultDevice.Id == device.Id) {
                deviceJson[U("default")] = web::json::value(true);
            }
            response[i++] = deviceJson;
        }
        request.reply(web::http::status_codes::OK, response);
    }
};
