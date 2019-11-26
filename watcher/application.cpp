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

#include <functional>
#include <loguru.hpp>
#include <string>
#include <thread>
#include <PathCch.h>
#include <Windows.h>

SERVICE_STATUS service_status;
SERVICE_STATUS_HANDLE status_handle;
HANDLE stop_event = nullptr;

#define SERVICE_NAME TEXT("twain-server-watcher")

dasa::twain::watcher::process process_watcher;
bool should_stop;

std::wstring get_current_working_dir()
{
    TCHAR path[MAX_PATH];

    if (!GetCurrentDirectory(MAX_PATH, path)) {
        return std::wstring();
    }

    return std::wstring(path);
}

std::wstring get_self_filename()
{
    using namespace std::literals;

    TCHAR path[MAX_PATH];

    if (!GetModuleFileName(nullptr, path, MAX_PATH)) {
        return L""s;
    }

    std::wstring filepath(path);
    if (filepath.find(' ') != std::wstring::npos) {
        filepath = L'"' + filepath + L'"';
    }

    return filepath;
}

struct SC_HANDLE_wrapper {
    SC_HANDLE_wrapper(SC_HANDLE handle) : handle(handle) {}
    ~SC_HANDLE_wrapper() {
        if (handle) {
            CloseServiceHandle(handle);
        }
        handle = nullptr;
    }

    operator SC_HANDLE() {
        return handle;
    }

private:
    SC_HANDLE handle;
};

SC_HANDLE_wrapper open_sc_manager(DWORD desired_access = SC_MANAGER_ALL_ACCESS)
{
    auto manager = OpenSCManager(nullptr, nullptr, desired_access);

    if (!manager) {
        return nullptr;
    }

    return manager;
}

void install_service() {
    auto manager = open_sc_manager(STANDARD_RIGHTS_REQUIRED | SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);
    auto filepath = get_self_filename() + L" " + get_current_working_dir();

    auto service = CreateService(manager, SERVICE_NAME, SERVICE_NAME,
        SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
        filepath.c_str(), nullptr, nullptr,
        nullptr, nullptr, nullptr);

    if (service != nullptr) {
        CloseServiceHandle(service);
    }
}

void remove_service() {
    auto manager = open_sc_manager();
    auto service = OpenService(manager, SERVICE_NAME, DELETE);
    if (!service) {
        return;
    }

    DeleteService(service);
    CloseServiceHandle(service);
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

void watch_child(std::wstring && command_line) {
    while (!should_stop) {
        if (!process_watcher.is_alive()) {
            process_watcher.reset();
            process_watcher.create_child(command_line);
        }

        process_watcher.join();
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

    wchar_t selfdir[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, selfdir, MAX_PATH);
    PathCchRemoveFileSpec(selfdir, MAX_PATH);

    // Initialization
    std::wstring command_line = selfdir + L"\\twain-server.exe"s;
    process_watcher.create_child(command_line);
    should_stop = false;

    report_status(SERVICE_RUNNING, NO_ERROR, 0);

    // Running
    watch_child(std::move(command_line));

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
