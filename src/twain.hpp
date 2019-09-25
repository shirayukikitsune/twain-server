#pragma once

#include <nlohmann/json.hpp>

#if defined(WIN32) || defined(WIN64) || defined (_WINDOWS)
#include <Windows.h>
#endif

#include "external/twain.h"
#include "twain/transfer.hpp"

namespace dasa::gliese::scanner {

    class Twain {
    public:
        ~Twain();

        void fillIdentity();
        void loadDSM(const char *path);
        void openDSM();
        void closeDSM();
        int getState() { return state; }
        void setState(int newState) { state = newState; }

        pTW_IDENTITY getIdentity() {
            return &identity;
        }

        std::list<TW_IDENTITY> listSources();
        TW_IDENTITY getDefaultDataSource();

        DSMENTRYPROC entry;

        TW_STATUS getStatus(TW_UINT16 rc);

        bool loadDataSource(TW_UINT32 id);
        bool enableDataSource(TW_HANDLE handle, bool showUI);
        pTW_IDENTITY getDataSouce() { return currentDS.get(); }
        bool closeDS();

        bool isUsingCallbacks() { return useCallbacks; }

        TW_UINT16 setCapability(TW_UINT16 capability, int value, TW_UINT16 type);
		TW_UINT16 setCapability(TW_UINT16 Cap, const pTW_FIX32 _pValue);

        std::unique_ptr<dasa::gliese::scanner::twain::Transfer> startScan();

        TW_HANDLE DSM_MemAllocate(TW_UINT32 size);
        void DSM_Free(TW_HANDLE memory);
        TW_MEMREF DSM_LockMemory(TW_HANDLE memory);
        void DSM_UnlockMemory(TW_HANDLE memory);

    private:
        TW_IDENTITY identity{};
        TW_ENTRYPOINT entrypoint{};
        TW_USERINTERFACE ui{};
#ifdef TWH_CMP_MSC
        HMODULE
#else
        void*
#endif
            DSM = nullptr;
        int state = 1;
        bool useCallbacks = false;

        std::unique_ptr<TW_IDENTITY> currentDS;

        static std::map<TW_UINT32, TW_MEMREF> map;

        bool getCurrent(TW_CAPABILITY *pCap, TW_UINT32& val);
        TW_INT16 getCapability(TW_CAPABILITY& _cap, TW_UINT16 _msg = MSG_GET);
    };

}

std::ostream& operator<<(std::ostream& os, const TW_IDENTITY& identity);
nlohmann::json deviceToJson(TW_IDENTITY device);
