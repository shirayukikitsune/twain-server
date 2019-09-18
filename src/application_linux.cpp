#include "application_linux.hpp"

#include <chrono>
#include <csignal>
#include <thread>

#include <loguru.hpp>

using dasa::gliese::scanner::linux::Application;

static dasa::gliese::scanner::linux::Application linux_application;
dasa::gliese::scanner::Application *application = &linux_application;

extern "C" void trapSigterm(int signal) {
    if (signal == SIGTERM) {
        LOG_S(INFO) << "Received SIGTERM, closing application";
        linux_application.stop();
    }
}

void Application::initialize(std::shared_ptr<http::Listener> listener) {
    LOG_SCOPE_FUNCTION(INFO);
    LOG_S(INFO) << "Initializing linux application";
    dasa::gliese::scanner::Application::initialize(listener);

    LOG_S(INFO) << "Installing SIGTERM handler";
    signal(SIGTERM, trapSigterm);

    LOG_S(INFO) << "Loading TWAIN DSM library";
    twain.loadDSM("/usr/local/lib/libtwaindsm.so");
}

void Application::run() {
    LOG_SCOPE_FUNCTION(INFO);
    LOG_S(INFO) << "Application running";

    listener->listen();
}
