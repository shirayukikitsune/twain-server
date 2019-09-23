#include <loguru.hpp>
#include "memory_transfer.hpp"
#include <cpprest/rawptrstream.h>

using namespace dasa::gliese::scanner::twain;

void MemoryTransfer::transfer() {
    LOG_SCOPE_FUNCTION(INFO);

    bool pendingTransfers = true;

    TW_IMAGEINFO imageInfo;
    TW_SETUPMEMXFER sourceBufferSize;
    int idx = 0;

    while (pendingTransfers) {
        LOG_SCOPE_F(INFO, "transfer #%i", idx++);
        LOG_S(INFO) << "Getting image info";
        memset(&imageInfo, 0, sizeof(TW_IMAGEINFO));
        twain->entry(twain->getIdentity(), twain->getDataSouce(), DG_IMAGE, DAT_IMAGEINFO, MSG_GET, &imageInfo);

        LOG_S(INFO) << "Getting buffer size";
        memset(&sourceBufferSize, 0, sizeof(TW_SETUPMEMXFER));
        twain->entry(twain->getIdentity(), twain->getDataSouce(), DG_CONTROL, DAT_SETUPMEMXFER, MSG_GET, &sourceBufferSize);

        TW_IMAGEMEMXFER memXferTemplate;
        memXferTemplate.Compression = TWON_DONTCARE16;
        memXferTemplate.BytesPerRow = TWON_DONTCARE32;
        memXferTemplate.Columns = TWON_DONTCARE32;
        memXferTemplate.Rows = TWON_DONTCARE32;
        memXferTemplate.XOffset = TWON_DONTCARE32;
        memXferTemplate.YOffset = TWON_DONTCARE32;
        memXferTemplate.BytesWritten = TWON_DONTCARE32;
        memXferTemplate.Memory.Flags = TWMF_APPOWNS | TWMF_POINTER;
        memXferTemplate.Memory.Length = sourceBufferSize.Preferred;

        TW_HANDLE buffer = twain->DSM_MemAllocate(sourceBufferSize.Preferred);
        memXferTemplate.Memory.TheMem = twain->DSM_LockMemory(buffer);

        TW_IMAGEMEMXFER memXferBuffer;
        bool scanStarted = false;
        TW_UINT16 rc;

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

            if (!scanStarted) {
                twain->setState(7);
                scanStarted = true;
            }

            concurrency::streams::rawptr_buffer rawBuf(reinterpret_cast<char *>(memXferBuffer.Memory.TheMem), memXferBuffer.Memory.Length);
            os.write(rawBuf, memXferBuffer.Memory.Length);
            os.flush();

            if (rc == TWRC_XFERDONE) {
                break;
            }
        }

        twain->DSM_UnlockMemory(buffer);
        twain->DSM_Free(buffer);

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
