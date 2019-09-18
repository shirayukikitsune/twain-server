#include <iostream>
#include <loguru.hpp>

#include "application.hpp"

extern dasa::gliese::scanner::Application *application;

int main(int argc, char **argv) {
    loguru::init(argc, argv, "-v");

    auto listener = std::make_shared<dasa::gliese::scanner::http::Listener>();

    std::map<std::string, dasa::gliese::scanner::http::Listener::handler_t> routes;

    routes["/status"] = [](h2o_handler_t *self, h2o_req_t *req) -> int {
        if (h2o_memis(req->method.base, req->method.len, H2O_STRLIT("GET"))) {
            req->res.status = 200;
            req->res.reason = "OK";
            h2o_add_header(&req->pool, &req->res.headers, H2O_TOKEN_CONTENT_TYPE, nullptr, H2O_STRLIT("application/json"));
            h2o_send_inline(req, H2O_STRLIT("{\"status\":\"UP\"}"));
            return 0;
        }

        return -1;
    };

    listener->initialize(routes, "127.0.0.1", 43456);

    LOG_F(INFO, "Opening HTTP listener on 127.0.0.1:43456");

    application->initialize(listener);

    LOG_F(INFO, "Application initialized");

    LOG_S(INFO) << "Connecting to TWAIN";
    application->getTwain().fillIdentity();
    application->getTwain().openDSM();

    LOG_S(INFO) << "Listing TWAIN devices";
    auto devices = application->getTwain().listSources();
    for (auto & device : devices) {
        LOG_S(INFO) << device;
    }

    auto device = application->getTwain().getDefaultDataSource();
    LOG_S(INFO) << "Default device: " << device;

    application->run();

    LOG_F(INFO, "Closing TWAIN DSM");
    application->getTwain().closeDSM();
}

std::ostream& operator<<(std::ostream& os, const TW_IDENTITY& identity) {
    os << "Device " << identity.Id;
    if (identity.Id != 0) {
        os << ": " << identity.Manufacturer << ", " << identity.ProductName << ", " << identity.ProductFamily;
    } else {
        os << " has no data";
    }
    return os;
}
