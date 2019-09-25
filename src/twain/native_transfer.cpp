#include <loguru.hpp>
#include "native_transfer.hpp"

using namespace dasa::gliese::scanner::twain;

TW_IMAGEINFO NativeTransfer::prepare() {
	LOG_S(INFO) << "Getting image info";

	TW_IMAGEINFO imageInfo;
	memset(&imageInfo, 0, sizeof(TW_IMAGEINFO));
	twain->entry(twain->getIdentity(), twain->getDataSouce(), DG_IMAGE, DAT_IMAGEINFO, MSG_GET, &imageInfo);

	twain->setState(7);

	return imageInfo;
}

bool NativeTransfer::transferOne(std::ostream& os) {
	LOG_SCOPE_FUNCTION(INFO);

	TW_MEMREF hImg = nullptr;

	LOG_S(INFO) << "Starting transfer";
	auto rc = twain->entry(twain->getIdentity(), twain->getDataSouce(), DG_IMAGE, DAT_IMAGENATIVEXFER, MSG_GET, &hImg);

	if (rc == TWRC_CANCEL) {
		LOG_S(WARNING) << "Cancelled transfer while trying to get data";
		return false;
	}
	if (rc == TWRC_FAILURE) {
		LOG_S(ERROR) << "Error while transfering data from DS";
		return false;
	}
	if (rc == TWRC_XFERDONE) {
		PBITMAPINFOHEADER pDIB = (PBITMAPINFOHEADER)twain->DSM_LockMemory(hImg);
		if (!pDIB) {
			LOG_S(ERROR) << "Failed to lock memory";
			return false;
		}

		DWORD paletteSize = 0;
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
			pDIB->biSizeImage = ((((pDIB->biWidth * pDIB->biBitCount) + 31) & ~31) / 8) * pDIB->biHeight;

			// If a compression scheme is used the result may infact be larger
			// Increase the size to account for this.
			if (pDIB->biCompression != 0)
			{
				pDIB->biSizeImage = (pDIB->biSizeImage * 3) / 2;
			}
		}

		int nImageSize = pDIB->biSizeImage + (sizeof(RGBQUAD) * paletteSize) + sizeof(BITMAPINFOHEADER);

		BITMAPFILEHEADER bmpFIH = { 0 };
		bmpFIH.bfType = ((WORD)('M' << 8) | 'B');
		bmpFIH.bfSize = nImageSize + sizeof(BITMAPFILEHEADER);
		bmpFIH.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD) * paletteSize);

		os.write(reinterpret_cast<char*>(&bmpFIH), sizeof(BITMAPFILEHEADER));
		os.write(reinterpret_cast<char*>(pDIB), nImageSize);
	}

	LOG_S(INFO) << "Transfer finished";

	twain->DSM_UnlockMemory(hImg);
	twain->DSM_Free(hImg);

	if (rc != TWRC_XFERDONE) {
		return false;
	}

	return true;
}
