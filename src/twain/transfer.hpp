/*
    twain-server: This project exposes the TWAIN DSM as a web API
    Copyright (C) 2019 "Diagnósticos da América S.A. <bruno.f@dasa.com.br>"

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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
		explicit Transfer(dasa::gliese::scanner::Twain* twain) : twain(twain) {}
		virtual ~Transfer() = default;

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
