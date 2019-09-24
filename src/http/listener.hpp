#pragma once

#include "router.hpp"

#include <boost/beast.hpp>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>

namespace dasa::gliese::scanner::http {
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
