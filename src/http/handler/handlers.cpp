#include "handlers.hpp"

#include "devices.cpp"
#include "scan.cpp"
#include "status.cpp"

#include <vector>
#include <memory>

using namespace dasa::gliese::scanner::http;
using handler::RouteHandler;
std::vector<std::unique_ptr<RouteHandler>> handlers;

void create_handlers() {
    handlers.clear();
    handlers.emplace_back(std::make_unique<StatusHandler>());
    handlers.emplace_back(std::make_unique<ScanHandler>());
    handlers.emplace_back(std::make_unique<DevicesHandler>());
}

void install_handlers(const std::shared_ptr<Listener> &listener) {
    for (auto & handler : handlers) {
        listener->add_handler(std::move(handler));
    }
}
