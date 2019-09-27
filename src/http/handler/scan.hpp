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
