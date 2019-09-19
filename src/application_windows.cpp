#include "application_windows.hpp"
#include "http/listener.hpp"

#include <loguru.hpp>

static dasa::gliese::scanner::windows::Application windows_application;
dasa::gliese::scanner::Application *application = &windows_application;

using dasa::gliese::scanner::windows::Application;

void Application::initialize(std::shared_ptr<dasa::gliese::scanner::http::Listener> listener) {
    LOG_SCOPE_FUNCTION(INFO);
    LOG_S(INFO) << "Initializing Windows application";
    dasa::gliese::scanner::Application::initialize(listener);

    LOG_S(INFO) << "Loading TWAIN DSM library";
    twain.loadDSM("C:\\Windows\\System32\\TWAINDSM.dll");
}

void Application::run() {
    LOG_SCOPE_FUNCTION(INFO);
    LOG_S(INFO) << "Application running";

    listener->listen();
}
