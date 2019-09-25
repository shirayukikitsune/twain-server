#pragma once

#include "handler.hpp"

namespace dasa::gliese::scanner::http::handler {
    class StatusHandler : public RouteHandler {
    public:
        [[nodiscard]] boost::beast::http::verb method() const override {
            return boost::beast::http::verb::get;
        }

        [[nodiscard]] boost::beast::string_view route() const override {
            return "/status";
        }

        boost::beast::http::response<boost::beast::http::dynamic_body> operator()(boost::beast::http::request<boost::beast::http::string_body>&& request) override;
    };
}
