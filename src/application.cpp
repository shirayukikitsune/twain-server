#include "http/listener.hpp"

#include "application.hpp"

using namespace dasa::gliese::scanner;

void Application::initialize(std::shared_ptr<http::Listener> httpListener) {
    this->listener = std::move(httpListener);
}
