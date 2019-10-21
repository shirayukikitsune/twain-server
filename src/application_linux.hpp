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

namespace dasa::gliese::scanner::linux
{
    /**
     * Linux and MacOS specific application code
     */
    class Application : public dasa::gliese::scanner::Application {
    public:
        Application();

        void initialize(std::shared_ptr<http::Listener> listener) override;
        void run() override;
        void stop() override;

        Twain& getTwain() override { return twain; }

    private:
        Twain twain;
    };
} // namespace dasa::gliese::scanner::linux
