#pragma once

#include "application.hpp"

namespace dasa::gliese::scanner::linux
{
    class Application : public dasa::gliese::scanner::Application {
    public:
        void initialize(std::shared_ptr<http::Listener> listener) override;
        void run() override;
        void stop() { shouldRun = false; }

        Twain& getTwain() override { return twain; }

    private:
        bool shouldRun = true;
        Twain twain;
    };
} // namespace dasa::gliese::scanner::linux
