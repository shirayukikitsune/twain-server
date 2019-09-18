#pragma once

#include <memory>
#include "twain.hpp"
#include "http/listener.hpp"

namespace dasa::gliese::scanner
{
    class Application {
    public:
        virtual void initialize(std::shared_ptr<http::Listener> httpListener) {
            this->listener = std::move(httpListener);
        }
        virtual void run() = 0;
        virtual Twain& getTwain() = 0;

    protected:
        std::shared_ptr<http::Listener> listener;
    };
}
