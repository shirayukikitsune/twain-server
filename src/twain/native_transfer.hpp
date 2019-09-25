#pragma once

#include "transfer.hpp"
#include "../twain.hpp"

#include <ostream>

namespace dasa::gliese::scanner::twain {
    class NativeTransfer : public Transfer {
    public:
        NativeTransfer(dasa::gliese::scanner::Twain *twain) : Transfer(twain) {}

		TW_IMAGEINFO prepare() final;
		bool transferOne(std::ostream& outputStream) final;
    };
}