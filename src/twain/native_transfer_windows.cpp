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

#include "native_transfer.hpp"

#include <loguru.hpp>

using namespace dasa::gliese::scanner::twain;

TW_IMAGEINFO NativeTransfer::prepare() {
	LOG_S(INFO) << "Getting image info";

	TW_IMAGEINFO imageInfo;
	memset(&imageInfo, 0, sizeof(TW_IMAGEINFO));
	twain->entry(twain->getIdentity(), twain->getDataSouce(), DG_IMAGE, DAT_IMAGEINFO, MSG_GET, reinterpret_cast<TW_MEMREF>(&imageInfo));

	twain->setState(7);

	return imageInfo;
}

#define BYTES_PERLINE_ALIGN4(width, bpp) (((((int)(width)*(bpp))+31)/32)*4)
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

        auto colorCount = pDIB->biBitCount == 1 ? 2 : pDIB->biBitCount == 8 ? 256 : 0;
        unsigned paletteSize = sizeof(RGBQUAD) * colorCount;
		pDIB->biSizeImage = BYTES_PERLINE_ALIGN4(pDIB->biWidth, pDIB->biBitCount) * pDIB->biHeight;

		uint64_t nImageSize = pDIB->biSizeImage + paletteSize + sizeof(BITMAPINFOHEADER);

		BITMAPFILEHEADER bmpFIH = { 0 };
		bmpFIH.bfType = 0x4d42;
		bmpFIH.bfSize = nImageSize + sizeof(BITMAPFILEHEADER);
		bmpFIH.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + paletteSize;

		os.write(reinterpret_cast<char*>(&bmpFIH), sizeof(BITMAPFILEHEADER));
		os.write(reinterpret_cast<char*>(pDIB), nImageSize);
        os.flush();
	}

	LOG_S(INFO) << "Transfer finished";

	twain->DSM_UnlockMemory(reinterpret_cast<TW_HANDLE>(hImg));
	twain->DSM_Free(reinterpret_cast<TW_HANDLE>(hImg));

    return rc == TWRC_XFERDONE;
}

std::string NativeTransfer::getTransferMIME() {
    return "image/bmp";
}

std::string NativeTransfer::getDefaultMIME() {
    return "image/bmp";
}
