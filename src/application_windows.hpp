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

#include "application.hpp"

#include <Windows.h>

namespace dasa::gliese::scanner::windows
{
    class Application : public dasa::gliese::scanner::Application {
    public:
        void initialize(std::shared_ptr<http::Listener> listener) override;
        void run() override;
        void stop() { shouldRun = false; }

        Twain& getTwain() override { return twain; }
        TW_HANDLE getParentWindow() override { return hwnd; }
        TW_MEMREF getParentWindowRef() override { return &hwnd; }

    private:
        bool shouldRun = true;
        Twain twain;
        HWND hwnd = nullptr;
    };

}

