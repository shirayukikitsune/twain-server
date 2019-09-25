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

	auto rc = twain->entry(twain->getIdentity(), twain->getDataSouce(), DG_CONTROL, DAT_PENDINGXFERS, MSG_ENDXFER, &pendingXfers);

	if (rc == TWRC_SUCCESS) {
		LOG_S(INFO) << "Pending images count: " << pendingXfers.Count;
		if (pendingXfers.Count == 0) {
			pendingTransfers = false;
		}
	}
	else {
		pendingTransfers = false;
	}
}

void Transfer::clearPending() {
	if (pendingTransfers) {
		TW_PENDINGXFERS pendxfers;
		memset(&pendxfers, 0, sizeof(pendxfers));

		twain->entry(twain->getIdentity(), twain->getDataSouce(), DG_CONTROL, DAT_PENDINGXFERS, MSG_ENDXFER, &pendxfers);

		// We need to get rid of any pending transfers
		if (pendxfers.Count != 0) {
			memset(&pendxfers, 0, sizeof(pendxfers));

			twain->entry(twain->getIdentity(), twain->getDataSouce(), DG_CONTROL, DAT_PENDINGXFERS, MSG_RESET, &pendxfers);
		}
	}

	twain->setState(5);
}
