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

dasa::gliese::scanner::windows::Application *windows_application;
dasa::gliese::scanner::Application *application;

using dasa::gliese::scanner::windows::Application;

Application::Application() : twain(getTwainIoContext()) {

}

void Application::initialize(std::shared_ptr<dasa::gliese::scanner::http::Listener> listener) {
    LOG_SCOPE_FUNCTION(INFO);
    LOG_S(INFO) << "Initializing Windows application";

    LOG_S(INFO) << "Verifying for another instance";
    application_handle = CreateEvent(nullptr, FALSE, FALSE, L"Global\\DasaTwainServer");
    if (!application_handle) {
        ABORT_S() << "Failed to create event";
        return;
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(application_handle);
        application_handle = nullptr;
        ABORT_S() << "Another instance of the application is already running";
        return;
    }

    dasa::gliese::scanner::Application::initialize(listener);

    LOG_S(INFO) << "Loading TWAIN DSM library";
    twain.loadDSM("TWAINDSM.dll");

    LOG_S(INFO) << "Creating main window";
    
    const wchar_t CLASS_NAME[] = L"Gliese Scanner";
    auto instance = GetModuleHandle(NULL);
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    hwnd = CreateWindowEx(0, CLASS_NAME, L"Gliese Scanner", 0, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, nullptr, nullptr, instance, nullptr);

    if (!hwnd) {
        ABORT_S() << "Failed to create main window";
        return;
    }

    ShowWindow(hwnd, SW_HIDE);
}

void Application::run() {
    LOG_SCOPE_FUNCTION(INFO);
    LOG_S(INFO) << "Application running";

    listener->start();

    myThreadId = GetCurrentThreadId();

    std::thread ioThread([&] { ioc.run(); });
    std::thread twainThread([&] { twain_ioc.run(); });

    MSG msg = { };
    BOOL result;
    while ((result = GetMessage(&msg, NULL, 0, 0)) != 0)
    {
        if (result > 0) {
            if (twain.getState() > 3) {
                TW_EVENT twEvent;

                twEvent.pEvent = (TW_MEMREF)&msg;
                twEvent.TWMessage = MSG_NULL;
                TW_UINT16  twRC = TWRC_NOTDSEVENT;
                twRC = twain(DG_CONTROL, DAT_EVENT, MSG_PROCESSEVENT, &twEvent);

                if (!twain.isUsingCallbacks() && twRC == TWRC_DSEVENT) {
                    // check for message from Source
                    switch (twEvent.TWMessage)
                    {
                    case MSG_XFERREADY:
                        twain.setState(6);
                        break;
                    case MSG_CLOSEDSREQ:
                    case MSG_CLOSEDSOK:
                        twain.reset();
                        break;
                    case MSG_NULL:
                        LOG_S(INFO) << "Got message from DSM: " << twEvent.TWMessage;
                        break;
                    default:
                        LOG_S(INFO) << "Got unknown message from DSM: " << twEvent.TWMessage;
                        break;
                    }

                    if (twEvent.TWMessage != MSG_NULL) {
                        continue;
                    }
                }
                if (twRC == TWRC_DSEVENT) {
                    continue;
                }
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    listener->stop();
    ioc.stop();
    twain_ioc.stop();
}

void Application::stop() {
    PostThreadMessage(myThreadId, WM_QUIT, 0, 0);
    CloseHandle(application_handle);
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
