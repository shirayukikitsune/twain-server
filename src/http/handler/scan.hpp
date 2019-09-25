#pragma once

#include "handler.hpp"

namespace dasa::gliese::scanner::http::handler {
    class PrepareScanHandler : public RouteHandler {
    public:
        [[nodiscard]] boost::beast::http::verb method() const override {
            return boost::beast::http::verb::post;
        }

        [[nodiscard]] boost::beast::string_view route() const override {
            return "/scan/prepare";
        }

        boost::beast::http::response<boost::beast::http::dynamic_body> operator()(boost::beast::http::request<boost::beast::http::string_body>&& request) override;
    };

	class HasNextScanHandler : public RouteHandler {
	public:
		[[nodiscard]] boost::beast::http::verb method() const override {
			return boost::beast::http::verb::get;
		}

		[[nodiscard]] boost::beast::string_view route() const override {
			return "/scan/has-next";
		}

		boost::beast::http::response<boost::beast::http::dynamic_body> operator()(boost::beast::http::request<boost::beast::http::string_body>&& request) override;
	};

	class NextImageDataScanHandler : public RouteHandler {
	public:
		[[nodiscard]] boost::beast::http::verb method() const override {
			return boost::beast::http::verb::get;
		}

		[[nodiscard]] boost::beast::string_view route() const override {
			return "/scan/prepare-next";
		}

		boost::beast::http::response<boost::beast::http::dynamic_body> operator()(boost::beast::http::request<boost::beast::http::string_body>&& request) override;
	};

	class NextScanHandler : public RouteHandler {
	public:
		[[nodiscard]] boost::beast::http::verb method() const override {
			return boost::beast::http::verb::get;
		}

		[[nodiscard]] boost::beast::string_view route() const override {
			return "/scan/next";
		}

		boost::beast::http::response<boost::beast::http::dynamic_body> operator()(boost::beast::http::request<boost::beast::http::string_body>&& request) override;
	};

	class EndScanHandler : public RouteHandler {
	public:
		[[nodiscard]] boost::beast::http::verb method() const override {
			return boost::beast::http::verb::post;
		}

		[[nodiscard]] boost::beast::string_view route() const override {
			return "/scan/end";
		}

		boost::beast::http::response<boost::beast::http::dynamic_body> operator()(boost::beast::http::request<boost::beast::http::string_body>&& request) override;
	};

	class ScanHandler : public RouteHandler {
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
