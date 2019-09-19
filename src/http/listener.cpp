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

#define ADD_ROUTE2(fn, method) void fn (const utility::string_t & path, requesthandler_t handler) { method ## Router.add_handler(path, handler); }
#define ADD_ROUTE(method) ADD_ROUTE2(Listener::##method, method)

ADD_ROUTE(get)
ADD_ROUTE(post)
ADD_ROUTE(put)
ADD_ROUTE(del)
ADD_ROUTE(head)
ADD_ROUTE(options)
ADD_ROUTE(trace)
ADD_ROUTE(merge)
ADD_ROUTE(connect)
ADD_ROUTE(patch)
