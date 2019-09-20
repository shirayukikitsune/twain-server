#include <cpprest/http_msg.h>
#include "../../application.hpp"
#include "handlers.hpp"

extern dasa::gliese::scanner::Application *application;

class StatusHandler : public dasa::gliese::scanner::http::RouteHandler {
    [[nodiscard]] web::http::method method() const final {
        return web::http::methods::GET;
    }

    [[nodiscard]] utility::string_t route() const final {
        return U("/status");
    }

    void operator()(const web::http::http_request& request) final {
        auto response = web::json::value::object();
        response[U("status")] = web::json::value(U("UP"));

        auto details = web::json::value::object();
        details[U("twain")] = web::json::value(application->getTwain().getState());

        response[U("details")] = web::json::value(details);
        request.reply(web::http::status_codes::OK, response);
    }
};
