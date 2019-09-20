#include "listener.hpp"
#include "../exception/http_exception.hpp"

#include <chrono>
#include <thread>
#include <loguru.hpp>
#include <cpprest/uri.h>

using namespace dasa::gliese::scanner::http;

void Listener::initialize(const utility::char_t *address) {
    LOG_S(INFO) << "Creating HTTP listener";
    listener = std::make_unique<web::http::experimental::listener::http_listener>(web::http::uri(address));
    LOG_S(INFO) << "Configuring routes";
    listener->support(web::http::methods::GET, std::bind(&Router::handle_request, &getRouter, std::placeholders::_1));
    listener->support(web::http::methods::POST, std::bind(&Router::handle_request, &postRouter, std::placeholders::_1));
    listener->support(web::http::methods::PUT, std::bind(&Router::handle_request, &putRouter, std::placeholders::_1));
    listener->support(web::http::methods::DEL, std::bind(&Router::handle_request, &delRouter, std::placeholders::_1));
    listener->support(web::http::methods::HEAD, std::bind(&Router::handle_request, &headRouter, std::placeholders::_1));
    listener->support(web::http::methods::OPTIONS, std::bind(&Router::handle_request, &optionsRouter, std::placeholders::_1));
    listener->support(web::http::methods::TRCE, std::bind(&Router::handle_request, &traceRouter, std::placeholders::_1));
    listener->support(web::http::methods::MERGE, std::bind(&Router::handle_request, &mergeRouter, std::placeholders::_1));
    listener->support(web::http::methods::CONNECT, std::bind(&Router::handle_request, &connectRouter, std::placeholders::_1));
    listener->support(web::http::methods::PATCH, std::bind(&Router::handle_request, &patchRouter, std::placeholders::_1));
    LOG_S(INFO) << "Opening handler";
    listener->open().wait();
}

void Listener::listen() {
    while (shouldRun) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void Listener::add_handler(std::unique_ptr<RouteHandler> && handler) {
    auto method = handler->method();
    if (method == web::http::methods::GET) {
        getRouter.add_handler(std::move(handler));
    }
    else if (method == web::http::methods::POST) {
        postRouter.add_handler(std::move(handler));
    }
    else if (method == web::http::methods::PUT) {
        putRouter.add_handler(std::move(handler));
    }
    else if (method == web::http::methods::DEL) {
        delRouter.add_handler(std::move(handler));
    }
    else if (method == web::http::methods::HEAD) {
        headRouter.add_handler(std::move(handler));
    }
    else if (method == web::http::methods::OPTIONS) {
        optionsRouter.add_handler(std::move(handler));
    }
    else if (method == web::http::methods::TRCE) {
        traceRouter.add_handler(std::move(handler));
    }
    else if (method == web::http::methods::MERGE) {
        mergeRouter.add_handler(std::move(handler));
    }
    else if (method == web::http::methods::CONNECT) {
        connectRouter.add_handler(std::move(handler));
    }
    else if (method == web::http::methods::PATCH) {
        patchRouter.add_handler(std::move(handler));
    }
}
