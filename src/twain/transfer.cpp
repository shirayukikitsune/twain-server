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

#include "transfer.hpp"

#include "../twain.hpp"

#include <loguru.hpp>

using namespace dasa::gliese::scanner::twain;

void Transfer::transferAll(std::ostream& outputStream) {
	LOG_SCOPE_FUNCTION(INFO);

	int idx = 0;

	while (pendingTransfers) {
		LOG_SCOPE_F(INFO, "transfer #%i", idx++);

		prepare();

		if (!transferOne(outputStream)) {
			break;
		}

		checkPending();
	}

	twain->setState(6);

	clearPending();
}


void Transfer::checkPending() {
	LOG_S(INFO) << "Checking for more images";
	TW_PENDINGXFERS pendingXfers;
	memset(&pendingXfers, 0, sizeof(TW_PENDINGXFERS));

	auto rc = twain->entry(twain->getIdentity(), twain->getDataSouce(), DG_CONTROL, DAT_PENDINGXFERS, MSG_ENDXFER, reinterpret_cast<TW_MEMREF>(&pendingXfers));

	if (rc == TWRC_SUCCESS) {
		LOG_S(INFO) << "Pending images count: " << pendingXfers.Count;
		if (pendingXfers.Count == 0) {
			pendingTransfers = false;
		}
		twain->setState(6);
	}
	else {
		pendingTransfers = false;
	}
}

void Transfer::clearPending() {
    TW_PENDINGXFERS pendxfers;
	if (pendingTransfers) {
		memset(&pendxfers, 0, sizeof(pendxfers));

		twain->entry(twain->getIdentity(), twain->getDataSouce(), DG_CONTROL, DAT_PENDINGXFERS, MSG_ENDXFER, reinterpret_cast<TW_MEMREF>(&pendxfers));
	}
    memset(&pendxfers, 0, sizeof(pendxfers));

    twain->entry(twain->getIdentity(), twain->getDataSouce(), DG_CONTROL, DAT_PENDINGXFERS, MSG_RESET, reinterpret_cast<TW_MEMREF>(&pendxfers));

	twain->setState(5);
}
