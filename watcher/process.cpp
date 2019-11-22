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

#include "process.h"

#include <algorithm>
#include <system_error>
#include <thread>
#include <utility>
#include <Windows.h>

using dasa::twain::watcher::process;

process::process()
{
    ZeroMemory(&process_information, sizeof(PROCESS_INFORMATION));
}

void process::reset()
{
    if (is_alive()) {
        stop();
    }

    ZeroMemory(&process_information, sizeof(PROCESS_INFORMATION));
}

void process::create_child(std::wstring& command_line)
{
    STARTUPINFO startup_info;
    ZeroMemory(&startup_info, sizeof(STARTUPINFO));
    startup_info.cb = sizeof(STARTUPINFO);

    auto success = CreateProcess(nullptr, command_line.data(), nullptr, nullptr, true, 0, nullptr, nullptr, &startup_info, &process_information);
    if (!success) {
        throw std::make_error_code(std::errc::no_child_process);
    }
}

bool process::is_alive()
{
    DWORD exit_code;
    if (process_information.dwProcessId == 0) {
        return false;
    }

    auto success = GetExitCodeProcess(process_information.hProcess, &exit_code);

    return success && exit_code == STILL_ACTIVE;
}

void process::join()
{
    if (!is_alive()) {
        return;
    }

    WaitForSingleObject(process_information.hProcess, INFINITE);
}

void process::stop()
{
    stop_gracefully();
    terminate();
}

void process::stop_gracefully(std::chrono::milliseconds wait_duration)
{
    using namespace std::chrono_literals;
    if (!is_alive()) {
        return;
    }

    PostThreadMessage(process_information.dwThreadId, WM_QUIT, 0, 0);

    while (wait_duration.count() > 0 && is_alive()) {
        wait_duration -= 10ms;
        std::this_thread::sleep_for(10ms);
    }
}

void process::terminate()
{
    if (!is_alive()) {
        return;
    }

    auto success = TerminateProcess(process_information.hProcess, 0);
    if (!success) {
        throw std::make_error_code(std::errc::operation_not_permitted);
    }
}

// Implementation from https://stackoverflow.com/a/39290139
HWND process::find_main_window()
{
    if (!is_alive()) {
        return nullptr;
    }

    std::pair<HWND, DWORD> params{ 0, process_information.dwProcessId };

    auto result = EnumWindows([](HWND wnd, LPARAM param) -> BOOL
    {
        auto params = reinterpret_cast<std::pair<HWND, DWORD>*>(param);

        DWORD process_id;
        if (GetWindowThreadProcessId(wnd, &process_id) && process_id == params->second && GetWindow(wnd, GW_OWNER) == nullptr) {
            SetLastError(-1);
            params->first = wnd;
            return false;
        }

        return true;
    }, reinterpret_cast<LPARAM>(&params));

    if (!result && GetLastError() == -1 && params.first) {
        return params.first;
    }

    return nullptr;
}
