#pragma once

#include "router.hpp"

#include <boost/asio/coroutine.hpp>
#include <boost/beast.hpp>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>

namespace dasa::gliese::scanner::http {
    class Listener : public boost::asio::coroutine, public std::enable_shared_from_this<Listener> {
    public:
        Listener(boost::beast::net::io_context &ioContext);

        void listen(const char *address, unsigned short port);

        void add_handler(std::unique_ptr<handler::RouteHandler> && handler);

        void start() {
            loop();
        }

        void stop() {
            shouldRun = false;
        }

        Router* getRouterForVerb(boost::beast::http::verb verb);

    private:
        void loop(boost::beast::error_code ec = {});

        boost::beast::net::io_context &ioContext;
        boost::asio::ip::tcp::acceptor acceptor;
        boost::asio::ip::tcp::socket socket;

        bool shouldRun = true;

#define DECLARE_ROUTER(a, b) Router a##Router = Router(boost::beast::http::verb::b)

        DECLARE_ROUTER(get, get);
        DECLARE_ROUTER(post, post);
        DECLARE_ROUTER(put, put);
        DECLARE_ROUTER(del, delete_);
        DECLARE_ROUTER(head, head);
        DECLARE_ROUTER(options, options);
        DECLARE_ROUTER(trace, trace);
        DECLARE_ROUTER(merge, merge);
        DECLARE_ROUTER(connect, connect);
        DECLARE_ROUTER(patch, patch);

#undef DECLARE_ROUTER
    };
}
