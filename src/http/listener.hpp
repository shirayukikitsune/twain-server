#pragma once

#include "cpprest/http_listener.h"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>

namespace dasa::gliese::scanner::http {
    class RouteHandler {
    public:
        [[nodiscard]] virtual web::http::method method() const = 0;
        [[nodiscard]] virtual utility::string_t route() const = 0;

        virtual void operator()(const web::http::http_request& request) = 0;
    };

    class Router {
    public:
        explicit Router(web::http::method method) : method(std::move(method)) {}

        void add_handler(std::unique_ptr<RouteHandler> && handler) {
            handlers[handler->route()] = std::move(handler);
        }

        void handle_request(const web::http::http_request& request) {
            auto handler = handlers.find(request.relative_uri().path());
            if (handler == handlers.end()) {
                request.reply(web::http::status_codes::NotFound, "not found");
                return;
            }

            (*handler->second)(request);
        }

    private:
        std::map<utility::string_t, std::unique_ptr<RouteHandler>> handlers;
        web::http::method method;
    };

    class Listener {
    public:

        void initialize(const utility::char_t *address);

        void add_handler(std::unique_ptr<RouteHandler> && handler);

        void listen();

        void stop() {
            shouldRun = false;
        }

    private:
        std::unique_ptr<web::http::experimental::listener::http_listener> listener;
        bool shouldRun = true;

#define DECLARE_ROUTER(a, b) Router a##Router = Router(web::http::methods::b)

        DECLARE_ROUTER(get, GET);
        DECLARE_ROUTER(post, POST);
        DECLARE_ROUTER(put, PUT);
        DECLARE_ROUTER(del, DEL);
        DECLARE_ROUTER(head, HEAD);
        DECLARE_ROUTER(options, OPTIONS);
        DECLARE_ROUTER(trace, TRCE);
        DECLARE_ROUTER(merge, MERGE);
        DECLARE_ROUTER(connect, CONNECT);
        DECLARE_ROUTER(patch, PATCH);

#undef DECLARE_ROUTER
    };
}
