#pragma once

#include "transfer.hpp"
#include "../twain.hpp"

#include <ostream>

namespace dasa::gliese::scanner::twain {
    class MemoryTransfer : public Transfer {
    public:
        MemoryTransfer(dasa::gliese::scanner::Twain *twain, concurrency::streams::ostream &outputStream) : twain(twain), os(outputStream) {}
        void transfer() final;

    private:
        Twain *twain;
        concurrency::streams::ostream &os;
    };
}