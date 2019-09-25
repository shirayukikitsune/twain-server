#pragma once

#include "application.hpp"

#include <Windows.h>

namespace dasa::gliese::scanner::windows
{
    class Application : public dasa::gliese::scanner::Application {
    public:
        void initialize(std::shared_ptr<http::Listener> listener) override;
        void run() override;
        void stop() { shouldRun = false; }

        Twain& getTwain() override { return twain; }
        TW_HANDLE getParentWindow() override { return hwnd; }
        TW_MEMREF getParentWindowRef() override { return &hwnd; }

    private:
        bool shouldRun = true;
        Twain twain;
        HWND hwnd = nullptr;
    };

}

