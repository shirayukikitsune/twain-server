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
#include "native_transfer.hpp"

using namespace dasa::gliese::scanner::twain;

TW_IMAGEINFO NativeTransfer::prepare() {
	LOG_S(INFO) << "Getting image info";

	TW_IMAGEINFO imageInfo;
	memset(&imageInfo, 0, sizeof(TW_IMAGEINFO));
	twain->entry(twain->getIdentity(), twain->getDataSouce(), DG_IMAGE, DAT_IMAGEINFO, MSG_GET, reinterpret_cast<TW_MEMREF>(&imageInfo));

	twain->setState(7);

	return imageInfo;
}

bool NativeTransfer::transferOne(std::ostream& os) {
	LOG_SCOPE_FUNCTION(INFO);

	TW_MEMREF hImg = nullptr;

	LOG_S(INFO) << "Sizeof BITMAPINFOHEADER: " << sizeof(BITMAPINFOHEADER);
	LOG_S(INFO) << "Sizeof BITMAPFILEHEADER: " << sizeof(BITMAPFILEHEADER);

	LOG_S(INFO) << "Starting transfer";
	auto rc = twain->entry(twain->getIdentity(), twain->getDataSouce(), DG_IMAGE, DAT_IMAGENATIVEXFER, MSG_GET, reinterpret_cast<TW_MEMREF>(&hImg));

	if (rc == TWRC_CANCEL) {
		LOG_S(WARNING) << "Cancelled transfer while trying to get data";
		return false;
	}
	if (rc == TWRC_FAILURE) {
		LOG_S(ERROR) << "Error while transfering data from DS";
		return false;
	}
	if (rc == TWRC_XFERDONE) {
		auto pDIB = (PBITMAPINFOHEADER)twain->DSM_LockMemory(reinterpret_cast<TW_HANDLE>(hImg));
		if (!pDIB) {
			LOG_S(ERROR) << "Failed to lock memory";
			return false;
		}

		unsigned paletteSize = 0;
		switch (pDIB->biBitCount) {
		case 1:
			paletteSize = 2;
			break;
		case 8:
			paletteSize = 256;
			break;
		case 24:
			break;
		default:
			assert(0); //Not going to work!
			break;
		}

		if (pDIB->biSizeImage == 0)
		{
			pDIB->biSizeImage = ((((pDIB->biWidth * pDIB->biBitCount) + 31U) & ~31U) / 8) * pDIB->biHeight;

			// If a compression scheme is used the result may infact be larger
			// Increase the size to account for this.
			if (pDIB->biCompression != 0)
			{
				pDIB->biSizeImage = (pDIB->biSizeImage * 3) / 2;
			}
		}

		uint64_t nImageSize = pDIB->biSizeImage + (sizeof(RGBQUAD) * paletteSize) + sizeof(BITMAPINFOHEADER);

		BITMAPFILEHEADER bmpFIH = { 0 };
		bmpFIH.bfType = 0x4d42;
		bmpFIH.bfSize = nImageSize + sizeof(BITMAPFILEHEADER);
		bmpFIH.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD) * paletteSize);

		os.write(reinterpret_cast<char*>(&bmpFIH), sizeof(BITMAPFILEHEADER));
		os.write(reinterpret_cast<char*>(pDIB), nImageSize);
	}

	LOG_S(INFO) << "Transfer finished";

	twain->DSM_UnlockMemory(reinterpret_cast<TW_HANDLE>(hImg));
	twain->DSM_Free(reinterpret_cast<TW_HANDLE>(hImg));

    return rc == TWRC_XFERDONE;
}

std::string NativeTransfer::getTransferMIME() {
    return "image/bmp";
}
