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
#include <FreeImage.h>

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

    TW_IMAGEINFO imageInfo;
    memset(&imageInfo, 0, sizeof(TW_IMAGEINFO));
    twain->entry(twain->getIdentity(), twain->getDataSouce(), DG_IMAGE, DAT_IMAGEINFO, MSG_GET, reinterpret_cast<TW_MEMREF>(&imageInfo));

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
	    // Use FreeImage to convert from TIFF to BMP
	    auto image = FreeImage_OpenMemory(reinterpret_cast<BYTE*>(hImg), -1);
	    auto bitmap = FreeImage_LoadFromMemory(FIF_TIFF, image);

        auto output = FreeImage_OpenMemory();
        auto format = FIF_BMP;
        if (outputMime == "image/jpeg") {
            format = FIF_JPEG;
        } else if (outputMime == "image/png") {
            format = FIF_PNG;
        } else if (outputMime == "image/webp") {
            format = FIF_WEBP;
        }

        FreeImage_SaveToMemory(format, bitmap, output);
        FreeImage_CloseMemory(image);
        FreeImage_SeekMemory(output, 0, SEEK_SET);

        BYTE* buf;
        DWORD bufSize;

        FreeImage_AcquireMemory(output, &buf, &bufSize);
		os.write(reinterpret_cast<char*>(buf), bufSize);
	}

	LOG_S(INFO) << "Transfer finished";

	twain->DSM_UnlockMemory(reinterpret_cast<TW_HANDLE>(hImg));
	twain->DSM_Free(reinterpret_cast<TW_HANDLE>(hImg));

    return rc == TWRC_XFERDONE;
}

std::string NativeTransfer::getTransferMIME() {
    return outputMime == "*/*" ? getDefaultMIME() : outputMime;
}

std::string NativeTransfer::getDefaultMIME() {
    return "image/bmp";
}
