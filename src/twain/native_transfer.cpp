#include <loguru.hpp>
#include "native_transfer.hpp"

using namespace dasa::gliese::scanner::twain;

#ifndef _WINDOWS
typedef struct tagBITMAPINFOHEADER {
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct tagRGBQUAD {
    uint8_t rgbBlue;
    uint8_t rgbGreen;
    uint8_t rgbRed;
    uint8_t rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPFILEHEADER {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BITMAPFILEHEADER;
#endif

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
		PBITMAPINFOHEADER pDIB = (PBITMAPINFOHEADER)twain->DSM_LockMemory(reinterpret_cast<TW_HANDLE>(hImg));
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

		int nImageSize = pDIB->biSizeImage + (sizeof(RGBQUAD) * paletteSize) + sizeof(BITMAPINFOHEADER);

		BITMAPFILEHEADER bmpFIH = { 0 };
		bmpFIH.bfType = ((uint16_t)('M' << 8U) | 'B');
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
