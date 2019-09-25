#include <loguru.hpp>
#include "memory_transfer.hpp"

using namespace dasa::gliese::scanner::twain;

TW_IMAGEINFO MemoryTransfer::prepare() {
	LOG_S(INFO) << "Getting image info";
	memset(&imageInfo, 0, sizeof(TW_IMAGEINFO));
	twain->entry(twain->getIdentity(), twain->getDataSouce(), DG_IMAGE, DAT_IMAGEINFO, MSG_GET, &imageInfo);

	LOG_S(INFO) << "Getting buffer size";
	memset(&sourceBufferSize, 0, sizeof(TW_SETUPMEMXFER));
	twain->entry(twain->getIdentity(), twain->getDataSouce(), DG_CONTROL, DAT_SETUPMEMXFER, MSG_GET, &sourceBufferSize);
	
	auto bufferSize = sourceBufferSize.Preferred;

	LOG_S(INFO) << "Creating memory transfer template (Buffer size of " << bufferSize << " bytes)";
	memset(&memXferTemplate, 0, sizeof(TW_IMAGEMEMXFER));
	memXferTemplate.Compression = TWON_DONTCARE16;
	memXferTemplate.BytesPerRow = TWON_DONTCARE32;
	memXferTemplate.Columns = TWON_DONTCARE32;
	memXferTemplate.Rows = TWON_DONTCARE32;
	memXferTemplate.XOffset = TWON_DONTCARE32;
	memXferTemplate.YOffset = TWON_DONTCARE32;
	memXferTemplate.BytesWritten = TWON_DONTCARE32;
	memXferTemplate.Memory.Flags = TWMF_APPOWNS | TWMF_POINTER;
	memXferTemplate.Memory.Length = bufferSize;

	twain->setState(7);

	return imageInfo;
}

bool MemoryTransfer::transferOne(std::ostream& os) {
	TW_IMAGEMEMXFER memXferBuffer;
	bool scanStarted = false;
	TW_UINT16 rc;

	buffer = twain->DSM_MemAllocate(memXferTemplate.Memory.Length);
	memXferTemplate.Memory.TheMem = twain->DSM_LockMemory(buffer);

	LOG_S(INFO) << "Starting transfer";
	while (true) {
		memcpy(&memXferBuffer, &memXferTemplate, sizeof(TW_IMAGEMEMXFER));
		memset(memXferBuffer.Memory.TheMem, 0, memXferBuffer.Memory.Length);

		rc = twain->entry(twain->getIdentity(), twain->getDataSouce(), DG_IMAGE, DAT_IMAGEMEMXFER, MSG_GET, &memXferBuffer);

		if (rc == TWRC_CANCEL) {
			LOG_S(WARNING) << "Cancelled transfer while trying to get data";
			break;
		}
		if (rc == TWRC_FAILURE) {
			LOG_S(ERROR) << "Error while transfering data from DS";
			break;
		}

		twain->setState(7);

		os.write(reinterpret_cast<char*>(memXferBuffer.Memory.TheMem), memXferBuffer.Memory.Length);
		os.flush();

		if (rc == TWRC_XFERDONE) {
			break;
		}
	}

	twain->DSM_UnlockMemory(buffer);
	twain->DSM_Free(buffer);
	buffer = nullptr;

	if (rc != TWRC_XFERDONE) {
		return false;
	}

	return true;
}
