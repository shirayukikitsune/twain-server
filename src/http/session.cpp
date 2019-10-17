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

#include "session.hpp"

#include "listener.hpp"

#include <loguru.hpp>

using namespace dasa::gliese::scanner::http;
namespace bh = boost::beast::http;

Session::Session(boost::asio::ip::tcp::socket&& socket, std::shared_ptr<Listener> listener)
    : stream(std::move(socket))
    , listener(std::move(listener))
    , sendFunction(*this) {}

#include <boost/asio/yield.hpp>

void Session::loop(bool close, boost::beast::error_code ec, std::size_t bytesTransferred) {
    boost::ignore_unused(bytesTransferred);

    reenter(*this) {
        for (;;) {
            request = {};

            stream.expires_after(std::chrono::seconds(30));

            yield boost::beast::http::async_read(stream, buffer, request, boost::beast::bind_front_handler(&Session::loop, shared_from_this(), false));

            if (ec == boost::beast::http::error::end_of_stream) {
                break;
            }

            if (ec) {
                if (ec != boost::beast::error::timeout) {
                    LOG_S(ERROR) << "Failed to read from HTTP session: " << ec.message();
                }
                return;
            }

            if (!listener->getRouterForVerb(request.method())) {
                yield sendFunction(makeBadRequestResponse("invalid HTTP method"));
            }
            else {
                yield sendFunction(listener->getRouterForVerb(request.method())->handle_request(std::move(request)));
            }

            response = nullptr;
        }

        stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
    }
}

#include <boost/asio/unyield.hpp>

bh::response<bh::string_body> Session::makeBadRequestResponse(boost::beast::string_view why) {
    bh::response<bh::string_body> res{ bh::status::bad_request, request.version() };
    res.set(bh::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(bh::field::content_type, "text/html");
    res.keep_alive(request.keep_alive());
    res.body() = std::string(why);
    res.prepare_payload();
    return res;
}
