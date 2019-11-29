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
#ifdef ENABLE_IMAGE_CONVERSION
#include <Magick++.h>
#include <Magick++/ResourceLimits.h>
#endif

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

    twain.dsm().path("/usr/local/lib/libtwaindsm.so");
}

void Application::run() {
    LOG_SCOPE_FUNCTION(INFO);
    LOG_S(INFO) << "Application running";

    listener->start();

    auto thread = std::thread([this] {
        ioc.run();
    });

    while (listener->is_running()) {
        if (twain_ioc.stopped()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            twain_ioc.restart();
        }
        twain_ioc.run();
    }

    thread.join();
}

void Application::stop() {
    listener->stop();
    ioc.stop();
    twain_ioc.stop();
}

int main(int argc, char **argv) {
    loguru::init(argc, argv);
#ifdef ENABLE_IMAGE_CONVERSION
    Magick::InitializeMagick(argv[0]);
    LOG_S(INFO) << "Magick++ limits:";
    LOG_S(INFO) << " - Width: " << Magick::ResourceLimits::width();
    LOG_S(INFO) << " - Height: " << Magick::ResourceLimits::height();
    LOG_S(INFO) << " - List length: " << Magick::ResourceLimits::listLength();
    LOG_S(INFO) << " - Area: " << Magick::ResourceLimits::area();
    LOG_S(INFO) << " - Memory: " << Magick::ResourceLimits::memory();
    LOG_S(INFO) << " - Map: " << Magick::ResourceLimits::map();
    LOG_S(INFO) << " - Disk: " << Magick::ResourceLimits::disk();
    LOG_S(INFO) << " - File: " << Magick::ResourceLimits::file();
    LOG_S(INFO) << " - Thread: " << Magick::ResourceLimits::thread();
    LOG_S(INFO) << " - Throttle: " << Magick::ResourceLimits::throttle();
#endif

    linux_application = new Application;
    application = linux_application;
    auto listener = std::make_shared<dasa::gliese::scanner::http::Listener>(application->getIoContext());

    listener->listen("127.0.0.1", 43456);

    LOG_F(INFO, "Opening HTTP listener on 127.0.0.1:43456");

    application->initialize(listener);

    LOG_F(INFO, "Application initialized");

    LOG_S(INFO) << "Filling TWAIN identity";
    application->getTwain().fillIdentity();

    application->run();
}
