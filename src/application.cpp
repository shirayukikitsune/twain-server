#include "http/listener.hpp"

#include "application.hpp"

#include "http/handler/handlers.hpp"

using namespace dasa::gliese::scanner;

void Application::initialize(std::shared_ptr<http::Listener> httpListener) {
    this->listener = std::move(httpListener);
    create_handlers();
    install_handlers(listener);
}
