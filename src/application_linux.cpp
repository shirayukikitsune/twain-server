#include "application_linux.hpp"

#include <csignal>
#include <thread>

#include <loguru.hpp>

using dasa::gliese::scanner::linux::Application;

dasa::gliese::scanner::linux::Application *linux_application;
dasa::gliese::scanner::Application *application;

extern "C" void trapStop(int signal) {
    if (signal == SIGTERM || signal == SIGINT) {
        LOG_S(INFO) << "Received SIGTERM, closing application";
        linux_application->stop();
    }
}

void Application::initialize(std::shared_ptr<http::Listener> listener) {
    LOG_SCOPE_FUNCTION(INFO);
    LOG_S(INFO) << "Initializing linux application";
    dasa::gliese::scanner::Application::initialize(listener);

    LOG_S(INFO) << "Installing SIGTERM and SIGINT handler";
    signal(SIGTERM, trapStop);
	signal(SIGINT, trapStop);

    LOG_S(INFO) << "Loading TWAIN DSM library";
    twain.loadDSM("/usr/local/lib/libtwaindsm.so");
}

void Application::run() {
    LOG_SCOPE_FUNCTION(INFO);
    LOG_S(INFO) << "Application running";

    listener->listen();
}

int main(int argc, char **argv) {
    loguru::init(argc, argv, "-v");

    linux_application = new Application;
    application = linux_application;
    auto listener = std::make_shared<dasa::gliese::scanner::http::Listener>();

    listener->initialize(U("http://127.0.0.1:43456"));

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
