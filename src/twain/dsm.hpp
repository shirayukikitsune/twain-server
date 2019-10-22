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

#if defined(WIN32) || defined(WIN64) || defined (_WINDOWS)
#include <Windows.h>
#endif

#include <memory>
#include <string>
#include "../external/twain.h"

namespace dasa::gliese::scanner::twain {

    class DSM {
    public:
        enum struct State {
            Unloaded,
            Disconnected,
            Ready,
        };

        void state(State state) {
            this->state_ = state;
        }
        State state() {
            return this->state_;
        }

        void path(std::string path) {
            dsm_lib_path = std::move(path);
        }

        [[nodiscard]] bool load();
        void unload();

        [[nodiscard]] bool open(pTW_IDENTITY identity, TW_MEMREF parent);
        void close(pTW_IDENTITY identity);

        TW_HANDLE alloc(TW_UINT32 size);
        void free(TW_HANDLE memory);
        TW_MEMREF lock(TW_HANDLE memory);
        void unlock(TW_HANDLE memory);

        TW_UINT16 operator()(pTW_IDENTITY pOrigin,
                             pTW_IDENTITY pDest,
                             TW_UINT32 DG,
                             TW_UINT16 DAT,
                             TW_UINT16 MSG,
                             TW_MEMREF pData);

    private:
#ifdef TWH_CMP_MSC
        typedef HMODULE module_t;
#else
        typedef void module_t;
#endif
        struct module_releaser {
            void operator()(module_t* module) const;
        };
        typedef std::unique_ptr<module_t, module_releaser> module_ptr_t;
        module_ptr_t dsm_module;

        TW_ENTRYPOINT entrypoint{};
        DSMENTRYPROC entry = nullptr;
        std::string dsm_lib_path = "";
        State state_ = State::Unloaded;
    };

}
