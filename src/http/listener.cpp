#include "listener.hpp"
#include "session.hpp"

#include <boost/asio/strand.hpp>
#include <chrono>
#include <thread>
#include <loguru.hpp>

using namespace dasa::gliese::scanner::http;

Listener::Listener(boost::beast::net::io_context &ioContext)
    : ioContext(ioContext)
    , acceptor(ioContext)
    , socket(ioContext) {}

void Listener::listen(const char *address, unsigned short port) {
    auto const netAddress = boost::beast::net::ip::make_address(address);
    boost::asio::ip::tcp::endpoint endpoint{ netAddress, port };

    LOG_S(INFO) << "Setting up handlers";

    auto serviceHandlers = kitsune::ioc::Injector<dasa::gliese::scanner::http::handler::RouteHandler>::getInstance().findServices();
    for (auto & handlerPtr : serviceHandlers) {
        auto handler = handlerPtr.lock();
        if (handler) {
            LOG_S(INFO) << "Found handler for path \"" << handler->route() << "\", method " << boost::beast::http::to_string(handler->method());
            add_handler(std::move(handler));
        }
    }

    LOG_S(INFO) << "Creating TCP acceptor";

    boost::beast::error_code ec;
    acceptor.open(endpoint.protocol(), ec);
    if (ec) {
        ABORT_S() << "Failed to open TCP acceptor: " << ec.message();
        return;
    }

    acceptor.set_option(boost::beast::net::socket_base::reuse_address(true), ec);
    if (ec) {
        ABORT_S() << "Failed to enable reuse address: " << ec.message();
        return;
    }

    acceptor.bind(endpoint, ec);
    if (ec) {
        ABORT_S() << "Failed to bind to endpoint (" << endpoint.address().to_string() << ":" << endpoint.port() << "): " << ec.message();
        return;
    }

    acceptor.listen(boost::beast::net::socket_base::max_listen_connections, ec);
    if (ec) {
        ABORT_S() << "Failed to listen: " << ec.message();
        return;
    }

    LOG_S(INFO) << "TCP acceptor created";
}

#define HANDLER_CASE(a, b) case boost::beast::http::verb::a : return &b##Router;
#define HANDLER_CASE1(a) HANDLER_CASE(a, a)

Router* Listener::getRouterForVerb(boost::beast::http::verb verb)
{
    switch (verb) {
        HANDLER_CASE1(get)
        HANDLER_CASE1(post)
        HANDLER_CASE1(put)
        HANDLER_CASE(delete_, del)
        HANDLER_CASE1(head)
        HANDLER_CASE1(options)
        HANDLER_CASE1(trace)
        HANDLER_CASE1(merge)
        HANDLER_CASE1(connect)
        HANDLER_CASE1(patch)
        default:
            return nullptr;
    }
}

#undef HANDLER_CASE1
#undef HANDLER_CASE

#include <boost/asio/yield.hpp>

void Listener::loop(boost::beast::error_code ec) {
    reenter(*this) {
        for (;;) {
            yield acceptor.async_accept(socket, boost::beast::bind_front_handler(&Listener::loop, shared_from_this()));

            if (ec) {
                LOG_S(ERROR) << "Failed to accept incoming connection: " << ec.message();
            }
            else {
                std::make_shared<dasa::gliese::scanner::http::Session>(std::move(socket), shared_from_this())->run();
            }

            socket = boost::asio::ip::tcp::socket(boost::asio::make_strand(ioContext));
        }
    }
}

#include <boost/asio/unyield.hpp>

#define HANDLER_CASE(a, b) case boost::beast::http::verb::a : b ## Router.add_handler(std::move(handler)); break;
#define HANDLER_CASE1(a) HANDLER_CASE(a, a)

void Listener::add_handler(std::shared_ptr<handler::RouteHandler> && handler) {
    auto method = handler->method();
    switch (method) {
        HANDLER_CASE1(get)
        HANDLER_CASE1(post)
        HANDLER_CASE1(put)
        HANDLER_CASE(delete_, del)
        HANDLER_CASE1(head)
        HANDLER_CASE1(options)
        HANDLER_CASE1(trace)
        HANDLER_CASE1(merge)
        HANDLER_CASE1(connect)
        HANDLER_CASE1(patch)
        default: break;
    }
}

#undef HANDLER_CASE1
#undef HANDLER_CASE
