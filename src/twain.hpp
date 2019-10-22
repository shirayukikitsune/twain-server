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

#include <boost/asio/execution_context.hpp>

namespace dasa::gliese::scanner {

    /**
     * Class that interfaces with the TWAIN DSM
     *
     * The DSM is a FSM, so we replicate it here. States are all integers, in range [1; 7].
     * Please refer to the TWAIN DSM reference for more details
     */
    class Twain {
    public:
        explicit Twain(boost::asio::io_context &context);
        ~Twain();

        // TWAIN in MacOS uses an uintptr_t for the ID, instead of an integer. We typedef that to avoid several #ifdefs
        /// The type of a device identifier
        typedef decltype(TW_IDENTITY::Id) TW_ID;

        /**
         * Fills the application identity struct
         */
        void fillIdentity();

        /**
         * @brief Loads the DSM library from the specified path
         *
         * If the state is 2 or greater, this method does nothing
         * If the state is 1, loads the library and state goes to 2
         *
         * @remark On MacOS this method does nothing, since the DSM is statically linked
         * @param path The path to the library
         */
        void loadDSM(const char *path);

        /**
         * @brief Unloads the DSM library
         */
        void unloadDSM();

        /**
         * @brief Opens a connection to the DSM
         *
         * If the state is 3 or greater, this method does nothing
         * If the state is 1, abort() is called
         * If the state is 2, then the connection is opened and state goes to 3
         *
         * @remark If the connection fails, then the state continues to be 2
         */
        void openDSM();

        /**
         * @brief Closes the DSM connection
         */
        void closeDSM();
        int getState() { return state; }
        void setState(int newState) { state = newState; }

        bool isUsingCallbacks() {
            return useCallbacks;
        }

        pTW_IDENTITY getIdentity() {
            return &identity;
        }

        std::list<TW_IDENTITY> listSources();
        TW_IDENTITY getDefaultDataSource();

        DSMENTRYPROC entry;

        TW_STATUSUTF8 getStatus(TW_UINT16 rc);

        bool loadDataSource(TW_ID id);

        void enableDataSource(TW_HANDLE handle, bool showUI);

        pTW_IDENTITY getDataSouce() { return currentDS.get(); }
        bool closeDS();
        void disableDS();

        TW_UINT16 setCapability(TW_UINT16 capability, int value, TW_UINT16 type);
		TW_UINT16 setCapability(TW_UINT16 Cap, const TW_FIX32* _pValue);
        TW_INT16 getCapability(TW_CAPABILITY& _cap, TW_UINT16 _msg = MSG_GET);

        std::unique_ptr<dasa::gliese::scanner::twain::Transfer> startScan(const std::string &outputMime);

        TW_HANDLE DSM_MemAllocate(TW_UINT32 size);
        void DSM_Free(TW_HANDLE memory);
        TW_MEMREF DSM_LockMemory(TW_HANDLE memory);
        void DSM_UnlockMemory(TW_HANDLE memory);

        pTW_IDENTITY getCurrentDS() { return currentDS.get(); }

    private:
        TW_IDENTITY identity{};
        TW_ENTRYPOINT entrypoint{};
        TW_USERINTERFACE ui{};

        boost::asio::io_context& context;

#ifdef TWH_CMP_MSC
        HMODULE
#else
        void*
#endif
            DSM = nullptr;
        int state = 1;
        bool useCallbacks = false;

        void shutdown() noexcept;

        std::unique_ptr<TW_IDENTITY> currentDS;

        static std::map<TW_UINT32, TW_MEMREF> map;

        bool getCurrent(TW_CAPABILITY *pCap, TW_UINT32& val);
    };

}

std::ostream& operator<<(std::ostream& os, const TW_IDENTITY& identity);
nlohmann::json deviceToJson(TW_IDENTITY device);
