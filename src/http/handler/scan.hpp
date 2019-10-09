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
    class PrepareScanHandler : public kitsune::ioc::Service<PrepareScanHandler, RouteHandler> {
    public:
        [[nodiscard]] boost::beast::http::verb method() const override {
            return boost::beast::http::verb::post;
        }

        [[nodiscard]] boost::beast::string_view route() const override {
            return "/scan/prepare";
        }

        boost::beast::http::response<boost::beast::http::dynamic_body> operator()(boost::beast::http::request<boost::beast::http::string_body>&& request) override;
    };

	class HasNextScanHandler : public kitsune::ioc::Service<HasNextScanHandler, RouteHandler> {
	public:
		[[nodiscard]] boost::beast::http::verb method() const override {
			return boost::beast::http::verb::get;
		}

		[[nodiscard]] boost::beast::string_view route() const override {
			return "/scan/has-next";
		}

		boost::beast::http::response<boost::beast::http::dynamic_body> operator()(boost::beast::http::request<boost::beast::http::string_body>&& request) override;
	};

	class NextImageDataScanHandler : public kitsune::ioc::Service<NextImageDataScanHandler, RouteHandler> {
	public:
		[[nodiscard]] boost::beast::http::verb method() const override {
			return boost::beast::http::verb::get;
		}

		[[nodiscard]] boost::beast::string_view route() const override {
			return "/scan/prepare-next";
		}

		boost::beast::http::response<boost::beast::http::dynamic_body> operator()(boost::beast::http::request<boost::beast::http::string_body>&& request) override;
	};

	class NextScanHandler : public kitsune::ioc::Service<NextScanHandler, RouteHandler> {
	public:
		[[nodiscard]] boost::beast::http::verb method() const override {
			return boost::beast::http::verb::get;
		}

		[[nodiscard]] boost::beast::string_view route() const override {
			return "/scan/next";
		}

		boost::beast::http::response<boost::beast::http::dynamic_body> operator()(boost::beast::http::request<boost::beast::http::string_body>&& request) override;
	};

	class EndScanHandler : public kitsune::ioc::Service<EndScanHandler, RouteHandler> {
	public:
		[[nodiscard]] boost::beast::http::verb method() const override {
			return boost::beast::http::verb::post;
		}

		[[nodiscard]] boost::beast::string_view route() const override {
			return "/scan/end";
		}

		boost::beast::http::response<boost::beast::http::dynamic_body> operator()(boost::beast::http::request<boost::beast::http::string_body>&& request) override;
	};

	class ScanHandler : public kitsune::ioc::Service<ScanHandler, RouteHandler> {
	public:
		[[nodiscard]] boost::beast::http::verb method() const override {
			return boost::beast::http::verb::post;
		}

		[[nodiscard]] boost::beast::string_view route() const override {
			return "/scan";
		}

		boost::beast::http::response<boost::beast::http::dynamic_body> operator()(boost::beast::http::request<boost::beast::http::string_body>&& request) override;
	};
}
