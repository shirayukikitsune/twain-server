#pragma once

#include "h2o.h"
#include <map>
#include <string>

namespace dasa::gliese::scanner::http {
    class Listener {
    public:
        typedef int (*handler_t)(h2o_handler_t *, h2o_req_t *);

        void initialize(const std::map<std::string, handler_t>& routes, const char *address, int port);

        void listen();

        h2o_accept_ctx_t* getAcceptContext() { return &acceptContext; }

    private:
        h2o_context_t context;
        uv_loop_t loop;
        h2o_globalconf_t config;
        h2o_accept_ctx_t acceptContext;
        uv_tcp_t uvListener;
    };
}
