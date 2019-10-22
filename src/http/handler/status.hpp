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

#include "handler.hpp"

namespace dasa::gliese::scanner::http::handler {
    class StatusHandler : public kitsune::ioc::Service<StatusHandler, RouteHandler> {
    public:
        [[nodiscard]] boost::beast::http::verb method() const override {
            return boost::beast::http::verb::get;
        }

        [[nodiscard]] boost::beast::string_view route() const override {
            return "/status";
        }

        boost::beast::http::response<boost::beast::http::dynamic_body> operator()(boost::beast::http::request<boost::beast::http::string_body>&& request) override;
    };

    class ResetHandler : public kitsune::ioc::Service<ResetHandler, RouteHandler> {
    public:
        [[nodiscard]] boost::beast::http::verb method() const override {
            return boost::beast::http::verb::delete_;
        }

        [[nodiscard]] boost::beast::string_view route() const override {
            return "/status";
        }

        boost::beast::http::response<boost::beast::http::dynamic_body> operator()(boost::beast::http::request<boost::beast::http::string_body>&& request) override;
    };
}
