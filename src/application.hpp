#pragma once

#include <boost/asio.hpp>
#include <memory>
#include "http/listener.hpp"
#include "twain.hpp"

namespace dasa::gliese::scanner
{
    class Application {
    public:
        virtual void initialize(std::shared_ptr<http::Listener> httpListener);
        virtual void run() = 0;
        virtual Twain& getTwain() = 0;

        boost::asio::io_context& getIoContext() { return ioc; }
        virtual TW_HANDLE getParentWindow() { return nullptr; }
        virtual TW_MEMREF getParentWindowRef() { return nullptr; }

    protected:
        std::shared_ptr<http::Listener> listener;
        boost::asio::io_context ioc;
    };
}
