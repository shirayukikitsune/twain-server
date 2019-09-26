#pragma once

#include "transfer.hpp"
#include "../twain.hpp"

#include <ostream>

namespace dasa::gliese::scanner::twain {
    class MemoryTransfer : public Transfer {
    public:
        explicit MemoryTransfer(dasa::gliese::scanner::Twain *twain) : Transfer(twain) {}

		TW_IMAGEINFO prepare() final;
        bool transferOne(std::ostream& outputStream) final;

    private:
		TW_IMAGEINFO imageInfo{};
		TW_SETUPMEMXFER sourceBufferSize{};
		TW_IMAGEMEMXFER memXferTemplate{};
		TW_HANDLE buffer = nullptr;
	};
}
