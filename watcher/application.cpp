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

#include "process.h"

#include <loguru.hpp>
#include <string>
#include <thread>
#include <PathCch.h>
#include <Windows.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

SERVICE_STATUS service_status;
SERVICE_STATUS_HANDLE status_handle;
HANDLE stop_event = nullptr;

#define SERVICE_NAME TEXT("twain-server-watcher")

dasa::twain::watcher::process process_watcher;
bool should_stop;

std::wstring get_current_working_dir() {
    TCHAR path[MAX_PATH];

    if (!GetCurrentDirectory(MAX_PATH, path)) {
        return std::wstring();
    }

    return std::wstring(path);
}

void install_service() {
    TCHAR path[MAX_PATH];

    if (!GetModuleFileName(nullptr, path, MAX_PATH)) {
        return;
    }

    auto manager = OpenSCManager(nullptr, nullptr, STANDARD_RIGHTS_REQUIRED | SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);

    if (manager == nullptr) {
        return;
    }

    std::wstring filepath(path);
    if (filepath.find(' ') != std::wstring::npos) {
        filepath = L'"' + filepath + L'"';
    }

    filepath += L" " + get_current_working_dir();

    auto service = CreateService(manager, SERVICE_NAME, SERVICE_NAME,
        SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
        filepath.c_str(), nullptr, nullptr,
        nullptr, nullptr, nullptr);
    if (service == nullptr) {
        CloseServiceHandle(manager);
        return;
    }

    CloseServiceHandle(service);
    CloseServiceHandle(manager);
}

void remove_service() {
    auto manager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (!manager) {
        return;
    }

    auto service = OpenService(manager, SERVICE_NAME, DELETE);
    if (!service) {
        CloseServiceHandle(manager);
        return;
    }

    DeleteService(service);

    CloseServiceHandle(service);
    CloseServiceHandle(manager);
}

void report_status(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint) {
    static DWORD check_point = 1;

    service_status.dwCurrentState = dwCurrentState;
    service_status.dwWin32ExitCode = dwWin32ExitCode;
    service_status.dwWaitHint = dwWaitHint;
    service_status.dwControlsAccepted = dwCurrentState == SERVICE_START_PENDING ? 0 : SERVICE_ACCEPT_STOP;

    if ((dwCurrentState == SERVICE_RUNNING) ||
        (dwCurrentState == SERVICE_STOPPED))
        service_status.dwCheckPoint = 0;
    else service_status.dwCheckPoint = check_point++;

    // Report the status of the service to the SCM.
    SetServiceStatus(status_handle, &service_status);
}

void WINAPI control_handler(DWORD control_code) {
    switch (control_code) {
    case SERVICE_CONTROL_STOP:
        report_status(SERVICE_STOP_PENDING, NO_ERROR, 0);

        should_stop = true;
        process_watcher.stop();
        report_status(service_status.dwCurrentState, NO_ERROR, 0);
        break;

    default:
        break;
    }
}

void WINAPI service_main(DWORD argc, LPTSTR* argv) {
    using namespace std::literals;

    status_handle = RegisterServiceCtrlHandler(SERVICE_NAME, control_handler);

    if (!status_handle) {
        //ABORT_F("Failed to register service control handler: %d", GetLastError());
        return;
    }

    service_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    service_status.dwServiceSpecificExitCode = 0;

    report_status(SERVICE_START_PENDING, NO_ERROR, 1500);

    stop_event = CreateEvent(nullptr, true, false, nullptr);
    if (stop_event == nullptr) {
        report_status(SERVICE_STOPPED, NO_ERROR, 0);
        return;
    }

    std::this_thread::sleep_for(20s);

    wchar_t selfdir[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, selfdir, MAX_PATH);
    PathCchRemoveFileSpec(selfdir, MAX_PATH);

    // Initialization
    std::wstring command_line = selfdir + L"\\twain-server.exe"s;
    process_watcher.create_child(command_line);
    should_stop = false;

    report_status(SERVICE_RUNNING, NO_ERROR, 0);

    // Running
    while (!should_stop) {
        if (!process_watcher.is_alive()) {
            process_watcher.reset();
            process_watcher.create_child(command_line);
        }

        process_watcher.join();
    }

    report_status(SERVICE_STOPPED, NO_ERROR, 0);
}

int main(int argc, char** argv) {
    loguru::init(argc, argv);

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-Install") == 0) {
            install_service();
            return 0;
        }
        if (strcmp(argv[i], "-Delete") == 0) {
            remove_service();
            return 0;
        }
    }

    SERVICE_TABLE_ENTRY serviceTable[] = {
        { SERVICE_NAME, service_main },
        { nullptr, nullptr },
    };

    if (!StartServiceCtrlDispatcher(serviceTable)) {
        ABORT_F("Startup failed with code %d", GetLastError());
        return 1;
    }
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
