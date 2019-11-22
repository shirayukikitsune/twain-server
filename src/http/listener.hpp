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

        void add_handler(std::shared_ptr<handler::RouteHandler> && handler);

        void start() {
            loop();
        }

        void stop();

        Router* getRouterForVerb(boost::beast::http::verb verb);

        bool is_running() {
            return shouldRun;
        }

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
