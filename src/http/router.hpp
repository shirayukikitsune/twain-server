#pragma once

#include "handler/handler.hpp"

#include <boost/beast.hpp>
#include <string>

namespace dasa::gliese::scanner::http {
	class Router {
	public:
		explicit Router(boost::beast::http::verb method) : method(std::move(method)) {}

		void add_handler(std::unique_ptr<handler::RouteHandler>&& handler) {
			handlers[handler->route()] = std::move(handler);
		}

		boost::beast::http::response<boost::beast::http::dynamic_body> handle_request(boost::beast::http::request<boost::beast::http::string_body>&& request);

	private:
		static boost::beast::http::response<boost::beast::http::dynamic_body> makeNotFoundResponse(const boost::beast::http::request<boost::beast::http::string_body>& request);

		std::map<boost::beast::string_view, std::unique_ptr<handler::RouteHandler>> handlers;
		boost::beast::http::verb method;
	};
}
