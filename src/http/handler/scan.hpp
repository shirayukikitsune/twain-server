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

#include "cors.hpp"
#include "handler.hpp"

namespace dasa::gliese::scanner::http::handler {
    class PrepareScanHandler : public kitsune::ioc::Service<PrepareScanHandler, PostMapping> {
    public:
        [[nodiscard]] boost::beast::string_view route() const override {
            return "/scan/prepare";
        }

        boost::beast::http::response<boost::beast::http::dynamic_body> operator()(boost::beast::http::request<boost::beast::http::string_body>&& request) override;
    };

    class PrepareScanCORSHandler : public CORSMapping<PrepareScanCORSHandler> {
    public:
        [[nodiscard]] boost::beast::string_view route() const override {
            return "/scan/prepare";
        }

    protected:
        [[nodiscard]] boost::beast::string_view methods() final {
            return "POST";
        }
    };

	class HasNextScanHandler : public kitsune::ioc::Service<HasNextScanHandler, GetMapping> {
	public:
		[[nodiscard]] boost::beast::string_view route() const override {
			return "/scan/has-next";
		}

		boost::beast::http::response<boost::beast::http::dynamic_body> operator()(boost::beast::http::request<boost::beast::http::string_body>&& request) override;
	};

    class HasNextScanCORSHandler : public CORSMapping<HasNextScanCORSHandler> {
    public:
        [[nodiscard]] boost::beast::string_view route() const override {
            return "/scan/has-next";
        }
    };

	class NextImageDataScanHandler : public kitsune::ioc::Service<NextImageDataScanHandler, GetMapping> {
	public:
		[[nodiscard]] boost::beast::string_view route() const override {
			return "/scan/prepare-next";
		}

		boost::beast::http::response<boost::beast::http::dynamic_body> operator()(boost::beast::http::request<boost::beast::http::string_body>&& request) override;
	};

    class NextImageDataScanCORSHandler : public CORSMapping<NextImageDataScanCORSHandler> {
    public:
        [[nodiscard]] boost::beast::string_view route() const override {
            return "/scan/prepare-next";
        }
    };

	class NextScanHandler : public kitsune::ioc::Service<NextScanHandler, GetMapping> {
	public:
		[[nodiscard]] boost::beast::string_view route() const override {
			return "/scan/next";
		}

		boost::beast::http::response<boost::beast::http::dynamic_body> operator()(boost::beast::http::request<boost::beast::http::string_body>&& request) override;
	};

    class NextScanCORSHandler : public CORSMapping<NextScanCORSHandler> {
    public:
        [[nodiscard]] boost::beast::string_view route() const override {
            return "/scan/next";
        }

    protected:
        [[nodiscard]] boost::beast::string_view headers() final {
            return "Server, Content-Type, X-Has-Next";
        }
    };

	class EndScanHandler : public kitsune::ioc::Service<EndScanHandler, PostMapping> {
	public:
		[[nodiscard]] boost::beast::string_view route() const override {
			return "/scan/end";
		}

		boost::beast::http::response<boost::beast::http::dynamic_body> operator()(boost::beast::http::request<boost::beast::http::string_body>&& request) override;
	};

    class EndScanCORSHandler : public CORSMapping<EndScanCORSHandler> {
    public:
        [[nodiscard]] boost::beast::string_view route() const override {
            return "/scan/end";
        }

    protected:
        [[nodiscard]] boost::beast::string_view methods() final {
            return "POST";
        }
    };

	class ScanHandler : public kitsune::ioc::Service<ScanHandler, PostMapping> {
	public:
		[[nodiscard]] boost::beast::string_view route() const override {
			return "/scan";
		}

		boost::beast::http::response<boost::beast::http::dynamic_body> operator()(boost::beast::http::request<boost::beast::http::string_body>&& request) override;
	};

    class ScanCORSHandler : public CORSMapping<ScanCORSHandler> {
    public:
        [[nodiscard]] boost::beast::string_view route() const override {
            return "/scan";
        }

    protected:
        [[nodiscard]] boost::beast::string_view methods() final {
            return "POST";
        }
    };
}
