#include <loguru.hpp>
#include "native_transfer.hpp"

using namespace dasa::gliese::scanner::twain;

void NativeTransfer::transfer() {
    LOG_SCOPE_FUNCTION(INFO);

    bool pendingTransfers = true;

    TW_IMAGEINFO imageInfo;
    int idx = 0;

    while (pendingTransfers) {
        LOG_SCOPE_F(INFO, "transfer #%i", idx++);
        LOG_S(INFO) << "Getting image info";
        memset(&imageInfo, 0, sizeof(TW_IMAGEINFO));
        twain->entry(twain->getIdentity(), twain->getDataSouce(), DG_IMAGE, DAT_IMAGEINFO, MSG_GET, &imageInfo);

        TW_MEMREF hImg = nullptr;

        LOG_S(INFO) << "Starting transfer";
        auto rc = twain->entry(twain->getIdentity(), twain->getDataSouce(), DG_IMAGE, DAT_IMAGENATIVEXFER, MSG_GET, &hImg);

        if (rc == TWRC_CANCEL) {
            LOG_S(WARNING) << "Cancelled transfer while trying to get data";
            break;
        }
        if (rc == TWRC_FAILURE) {
            LOG_S(ERROR) << "Error while transfering data from DS";
            break;
        }
        if (rc == TWRC_XFERDONE) {
            PBITMAPINFOHEADER pDIB = (PBITMAPINFOHEADER)twain->DSM_LockMemory(hImg);
            if (!pDIB) {
                LOG_S(ERROR) << "Failed to lock memory";
                break;
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

            int nImageSize = pDIB->biSizeImage + (sizeof(RGBQUAD)*paletteSize) + sizeof(BITMAPINFOHEADER);

            BITMAPFILEHEADER bmpFIH = { 0 };
            bmpFIH.bfType = ((WORD)('M' << 8) | 'B');
            bmpFIH.bfSize = nImageSize + sizeof(BITMAPFILEHEADER);
            bmpFIH.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD)*paletteSize);

            os.write(reinterpret_cast<char*>(&bmpFIH), sizeof(BITMAPFILEHEADER));
            os.write(reinterpret_cast<char*>(pDIB), nImageSize);
        }

        twain->DSM_UnlockMemory(hImg);
        twain->DSM_Free(hImg);

        if (rc != TWRC_XFERDONE) {
            break;
        }

        LOG_S(INFO) << "Checking for more images";
        TW_PENDINGXFERS pendingXfers;
        memset(&pendingXfers, 0, sizeof(TW_PENDINGXFERS));

        rc = twain->entry(twain->getIdentity(), twain->getDataSouce(), DG_CONTROL, DAT_PENDINGXFERS, MSG_ENDXFER, &pendingXfers);

        if (rc == TWRC_SUCCESS) {
            LOG_S(INFO) << "Pending images count: " << pendingXfers.Count;
            if (pendingXfers.Count == 0) {
                pendingTransfers = false;
            }
        } else {
            pendingTransfers = false;
        }
    }

    if (pendingTransfers) {
        TW_PENDINGXFERS pendxfers;
        memset(&pendxfers, 0, sizeof(pendxfers));

        twain->entry(twain->getIdentity(), twain->getDataSouce(), DG_CONTROL, DAT_PENDINGXFERS, MSG_ENDXFER, &pendxfers);

        // We need to get rid of any pending transfers
        if (pendxfers.Count != 0) {
            memset(&pendxfers, 0, sizeof(pendxfers));

            twain->entry(twain->getIdentity(), twain->getDataSouce(), DG_CONTROL, DAT_PENDINGXFERS, MSG_RESET, (TW_MEMREF)&pendxfers);
        }
    }

    twain->setState(5);
}
