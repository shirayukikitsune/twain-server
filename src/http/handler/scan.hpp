#pragma once

#include "handler.hpp"

namespace dasa::gliese::scanner::http::handler {
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
