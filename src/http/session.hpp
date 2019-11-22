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

#include <boost/asio/coroutine.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <memory>

namespace dasa::gliese::scanner::http {
    class Listener;

    class Session : public boost::asio::coroutine, public std::enable_shared_from_this<Session> {
        struct send {
            std::shared_ptr<Session> session;
            std::shared_ptr<void> response;

            template <bool isRequest, class Body, class Fields>
            void operator()(boost::beast::http::message<isRequest, Body, Fields>&& message) const {
                auto messagePtr = std::make_shared<boost::beast::http::message<isRequest, Body, Fields>>(std::move(message));

                session->response = messagePtr;

                boost::beast::http::async_write(session->stream, *messagePtr, boost::beast::bind_front_handler(&Session::loop, session->shared_from_this(), messagePtr->need_eof()));
            }
        };

        boost::beast::tcp_stream stream;
        boost::beast::flat_buffer buffer;
        boost::beast::http::request<boost::beast::http::string_body> request;
        std::shared_ptr<void> response;
        send sendFunction;
        std::shared_ptr<Listener> listener;

        boost::beast::http::response<boost::beast::http::string_body> makeBadRequestResponse(boost::beast::string_view why);

    public:
        explicit Session(boost::asio::ip::tcp::socket&& socket, std::shared_ptr<Listener> listener);

        void run() {
            loop(false, {}, 0);
        }

        void loop(bool close, boost::beast::error_code ec, std::size_t bytesTransferred);
    };
}
