#pragma ide diagnostic ignored "OCDFAInspection"
#pragma ide diagnostic ignored "cppcoreguidelines-pro-type-member-init"

#include "listener.hpp"
#include "../exception/http_exception.hpp"

#include <loguru.hpp>

using dasa::gliese::scanner::http::Listener;

static void handle_accept(uv_stream_t *listener, int status) {
    if (status != 0) {
        return;
    }

    auto conn = reinterpret_cast<uv_tcp_t*>(h2o_mem_alloc(sizeof(uv_tcp_t)));
    uv_tcp_init(listener->loop, conn);

    if (uv_accept(listener, reinterpret_cast<uv_stream_t*>(conn)) != 0) {
        uv_close(reinterpret_cast<uv_handle_t *>(conn), reinterpret_cast<uv_close_cb>(free));
        return;
    }

    auto sock = h2o_uv_socket_create(reinterpret_cast<uv_handle_t *>(conn), reinterpret_cast<uv_close_cb>(free));
    h2o_accept(reinterpret_cast<Listener*>(listener->data)->getAcceptContext(), sock);
}

void Listener::initialize(const std::map<std::string, handler_t>& routes, const char *address, int port) {
    h2o_config_init(&config);
    auto hostconf = h2o_config_register_host(&config, h2o_iovec_init(H2O_STRLIT("default")), 65535);

    for (auto & route : routes) {
        auto pathconf = h2o_config_register_path(hostconf, route.first.c_str(), 0);
        auto h2oHandler = h2o_create_handler(pathconf, sizeof(h2o_handler_t));
        h2oHandler->on_req = route.second;
    }

    uv_loop_init(&loop);
    h2o_context_init(&context, &loop, &config);

    acceptContext.ctx = &context;
    acceptContext.hosts = config.hosts;

    sockaddr_in addr;

    uv_tcp_init(context.loop, &uvListener);
    uv_ip4_addr(address, port, &addr);

    int resultCode;
    if ((resultCode = uv_tcp_bind(&uvListener, (sockaddr*)&addr, 0)) != 0) {
        throw dasa::gliese::scanner::exception::HTTPBindException(address, port, resultCode);
    }

    uvListener.data = this;
    if ((resultCode = uv_listen(reinterpret_cast<uv_stream_t *>(&uvListener), 128, handle_accept)) != 0) {
        throw dasa::gliese::scanner::exception::HTTPListenException(resultCode);
    }
}

void Listener::listen() {
    uv_run(context.loop, UV_RUN_DEFAULT);
}
