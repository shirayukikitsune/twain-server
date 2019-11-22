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

#pragma once

#include <chrono>
#include <string>
#include <Windows.h>

namespace dasa::twain::watcher {

    class process {
    public:
        process();

        void reset();
        void create_child(std::wstring& command_line);
        
        bool is_alive();
        void join();

        void stop();

        void stop_gracefully(std::chrono::milliseconds wait_duration = std::chrono::milliseconds(10'000));
        void terminate();

    private:
        PROCESS_INFORMATION process_information;

        HWND find_main_window();
    };

}
