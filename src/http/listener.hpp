#pragma once

#include "cpprest/http_listener.h"

#include <map>
#include <memory>
#include <string>

namespace dasa::gliese::scanner::http {
    typedef void(*requesthandler_t)(const web::http::http_request&);

    class Router {
    public:
        Router(web::http::method method) : method(method) {}

        void add_handler(utility::string_t path, requesthandler_t handler) {
            handlers[path] = handler;
        }

        void handle_request(web::http::http_request request) {
            auto handler = handlers.find(request.relative_uri().path());
            if (handler == handlers.end()) {
                request.reply(web::http::status_codes::NotFound, "not found");
                return;
            }

            handler->second(request);
        }

    private:
        std::map<utility::string_t, requesthandler_t> handlers;
        web::http::method method;
    };

    class Listener {
    public:

        void initialize(const utility::char_t *address);

        void get(const utility::string_t &path, requesthandler_t handler);
        void post(const utility::string_t &path, requesthandler_t handler);
        void put(const utility::string_t &path, requesthandler_t handler);
        void del(const utility::string_t &path, requesthandler_t handler);
        void head(const utility::string_t &path, requesthandler_t handler);
        void options(const utility::string_t &path, requesthandler_t handler);
        void trace(const utility::string_t &path, requesthandler_t handler);
        void merge(const utility::string_t &path, requesthandler_t handler);
        void connect(const utility::string_t &path, requesthandler_t handler);
        void patch(const utility::string_t &path, requesthandler_t handler);

        void listen();

        void stop() {
            shouldRun = false;
        }

    private:
        std::unique_ptr<web::http::experimental::listener::http_listener> listener;
        bool shouldRun = true;

#define DECLARE_ROUTER(a, b) Router a##Router = Router(web::http::methods::##b)

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
