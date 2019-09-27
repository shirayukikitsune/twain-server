#pragma once

#include "handler.hpp"

namespace dasa::gliese::scanner::http::handler {
    class DevicesHandler : public kitsune::ioc::Service<DevicesHandler, GetMapping> {
    public:
        [[nodiscard]] boost::beast::string_view route() const override {
            return "/devices";
        }

        boost::beast::http::response<boost::beast::http::dynamic_body> operator()(boost::beast::http::request<boost::beast::http::string_body>&& request) override;
    };
}

