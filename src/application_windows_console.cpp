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

#ifndef UNICODE
#define UNICODE
#endif

#include "application_windows.hpp"
#include "http/listener.hpp"

#include <loguru.hpp>
#include <shellapi.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

using dasa::gliese::scanner::windows::Application;
extern dasa::gliese::scanner::windows::Application *windows_application;
extern dasa::gliese::scanner::Application *application;

int main(int argc, char **argv) {
    loguru::init(argc, argv, "-v");

	windows_application = new Application;
	application = windows_application;
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
	for (auto& device : devices) {
		LOG_S(INFO) << device;
	}

	auto device = application->getTwain().getDefaultDataSource();
	LOG_S(INFO) << "Default device: " << device;

	application->run();

	LOG_F(INFO, "Closing TWAIN DSM");
	application->getTwain().closeDSM();
	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
