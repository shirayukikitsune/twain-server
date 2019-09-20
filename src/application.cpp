#include "application.hpp"

#include "http/handler/handlers.hpp"

using dasa::gliese::scanner::Application;

void Application::initialize(std::shared_ptr<http::Listener> httpListener) {
    this->listener = std::move(httpListener);
    create_handlers();
    install_handlers(listener);
}
