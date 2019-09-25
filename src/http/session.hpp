#pragma once

#include <boost/asio/coroutine.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <memory>

namespace dasa::gliese::scanner::http {
    class Listener;

    class Session : public boost::asio::coroutine, public std::enable_shared_from_this<Session> {
        struct send {
            Session &session;
            std::shared_ptr<void> response;

            explicit send(Session &session) : session(session) {}

            template <bool isRequest, class Body, class Fields>
            void operator()(boost::beast::http::message<isRequest, Body, Fields>&& message) const {
                auto messagePtr = std::make_shared<boost::beast::http::message<isRequest, Body, Fields>>(std::move(message));

                session.response = messagePtr;

                boost::beast::http::async_write(session.stream, *messagePtr, boost::beast::bind_front_handler(&Session::loop, session.shared_from_this(), messagePtr->need_eof()));
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
