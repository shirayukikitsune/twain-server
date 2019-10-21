/*
    twain-server: This project exposes the TWAIN DSM as a web API
    Copyright (C) 2019 "Diagnósticos da América S.A. <bruno.f@dasa.com.br>"

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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

Application::Application() : twain(getTwainIoContext()) {}

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

    listener->start();

    auto thread = std::thread([this] {
        ioc.run();
    });

    twain_ioc.run();

    thread.join();
}

void Application::stop() {
    listener->stop();
    ioc.stop();
    twain_ioc.stop();
}

int main(int argc, char **argv) {
    loguru::init(argc, argv, "-v");

    linux_application = new Application;
    application = linux_application;
    auto listener = std::make_shared<dasa::gliese::scanner::http::Listener>(application->getIoContext());

    listener->listen("127.0.0.1", 43456);

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
