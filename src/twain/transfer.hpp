#pragma once

#if defined(WIN32) || defined(WIN64) || defined (_WINDOWS)
#include <Windows.h>
#endif

#include "../external/twain.h"

#include <ostream>

namespace dasa::gliese::scanner {
	class Twain;
}

namespace dasa::gliese::scanner::twain {
    class Transfer {
    public:
		Transfer(dasa::gliese::scanner::Twain* twain) : twain(twain) {}

		virtual TW_IMAGEINFO prepare() = 0;
        void transferAll(std::ostream& outputStream);
		virtual bool transferOne(std::ostream& outputStream) = 0;
		virtual void end() {}
		
		void checkPending();
		void clearPending();
		bool hasPending() { return pendingTransfers; }

	protected:
		dasa::gliese::scanner::Twain* twain;
		bool pendingTransfers = true;
    };
}
