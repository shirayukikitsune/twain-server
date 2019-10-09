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

#include <loguru.hpp>
#include "memory_transfer.hpp"

using namespace dasa::gliese::scanner::twain;

TW_IMAGEINFO MemoryTransfer::prepare() {
	LOG_S(INFO) << "Getting image info";
	memset(&imageInfo, 0, sizeof(TW_IMAGEINFO));
	twain->entry(twain->getIdentity(), twain->getDataSouce(), DG_IMAGE, DAT_IMAGEINFO, MSG_GET, reinterpret_cast<TW_MEMREF>(&imageInfo));

	LOG_S(INFO) << "Getting buffer size";
	memset(&sourceBufferSize, 0, sizeof(TW_SETUPMEMXFER));
	twain->entry(twain->getIdentity(), twain->getDataSouce(), DG_CONTROL, DAT_SETUPMEMXFER, MSG_GET, reinterpret_cast<TW_MEMREF>(&sourceBufferSize));

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

		rc = twain->entry(twain->getIdentity(), twain->getDataSouce(), DG_IMAGE, DAT_IMAGEMEMXFER, MSG_GET, reinterpret_cast<TW_MEMREF>(&memXferBuffer));

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

    return rc == TWRC_XFERDONE;

}
