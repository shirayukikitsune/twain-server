#pragma once

#include "transfer.hpp"
#include "../twain.hpp"

#include <ostream>

namespace dasa::gliese::scanner::twain {
    class NativeTransfer : public Transfer {
    public:
        NativeTransfer(dasa::gliese::scanner::Twain *twain, std::ostream &outputStream) : twain(twain), os(outputStream) {}
        void transfer() final;

    private:
        Twain *twain;
        std::ostream &os;
    };
}