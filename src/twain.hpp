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

#pragma once

#include <nlohmann/json.hpp>
#include <list>

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

#ifdef __APPLE__
        bool loadDataSource(TW_MEMREF id);
#else
        bool loadDataSource(TW_UINT32 id);
#endif
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
