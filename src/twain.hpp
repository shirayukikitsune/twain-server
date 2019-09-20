#pragma once

#include <boost/dll.hpp>
#include <boost/function.hpp>
#include <cpprest/json.h>

#if defined(WIN32) || defined(WIN64) || defined (_WINDOWS)
#include <Windows.h>
#endif

#include "external/twain.h"

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

        boost::function<TW_UINT16(pTW_IDENTITY, pTW_IDENTITY, TW_UINT32, TW_UINT16, TW_UINT16, TW_MEMREF)> entry;

        TW_STATUSUTF8 getStatus(TW_UINT16 rc);

        void loadDataSource(TW_UINT32 id);
        void enableDataSource(TW_HANDLE handle, bool showUI);
        pTW_IDENTITY getDataSouce() { return currentDS.get(); }

        TW_UINT16 setCapability(TW_UINT16 capability, int value, TW_UINT16 type);

        void startScan();

        TW_HANDLE DSM_MemAllocate(TW_UINT32 size);
        void DSM_Free(TW_HANDLE memory);
        TW_MEMREF DSM_LockMemory(TW_HANDLE memory);
        void DSM_UnlockMemory(TW_HANDLE memory);

    private:
        TW_IDENTITY identity{};
        TW_ENTRYPOINT entrypoint{};
        TW_USERINTERFACE ui{};
        boost::dll::shared_library DSM;
        int state = 1;
        bool useCallbacks = false;

        std::unique_ptr<TW_IDENTITY> currentDS;

        static std::map<TW_UINT32, TW_MEMREF> map;

        bool getCurrent(TW_CAPABILITY *pCap, TW_UINT32& val);
        TW_INT16 getCapability(TW_CAPABILITY& _cap, TW_UINT16 _msg = MSG_GET);
    };

}

std::ostream& operator<<(std::ostream& os, const TW_IDENTITY& identity);
web::json::value deviceToJson(TW_IDENTITY device);
