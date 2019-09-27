#pragma once

#include <boost/beast.hpp>
#include <kitsune/ioc/service>
#include <string>

namespace dasa::gliese::scanner::http::handler {
    class RouteHandler : public kitsune::ioc::ServiceBase<RouteHandler> {
	public:
	    virtual ~RouteHandler() = default;
		[[nodiscard]] virtual boost::beast::http::verb method() const = 0;
		[[nodiscard]] virtual boost::beast::string_view route() const = 0;

        virtual boost::beast::http::response<boost::beast::http::dynamic_body> operator()(boost::beast::http::request<boost::beast::http::string_body> && request) = 0;
	};
}
