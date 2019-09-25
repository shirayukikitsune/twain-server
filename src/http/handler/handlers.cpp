#include "handlers.hpp"

#include "devices.hpp"
#include "scan.hpp"
#include "status.hpp"

#include <vector>
#include <memory>

using namespace dasa::gliese::scanner::http;
using handler::RouteHandler;
std::vector<std::unique_ptr<RouteHandler>> handlers;

void create_handlers() {
    handlers.clear();
    handlers.emplace_back(std::make_unique<handler::DevicesHandler>());
    handlers.emplace_back(std::make_unique<handler::ScanHandler>());
    handlers.emplace_back(std::make_unique<handler::StatusHandler>());
}

void install_handlers(const std::shared_ptr<Listener> &listener) {
    for (auto & handler : handlers) {
        listener->add_handler(std::move(handler));
    }
}
