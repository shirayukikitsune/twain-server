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

#include "dsm.hpp"

#include <algorithm>
#include <loguru.hpp>

#ifdef TWH_CMP_GNU
#include <dlfcn.h>
#define LOADLIBRARY(lib) dlopen(lib, RTLD_NOW)
#define LOADFUNCTION(lib, func) dlsym(lib, func)
#define UNLOADLIBRARY(lib) dlclose(lib)
#else
#define LOADLIBRARY(lib) LoadLibraryA(lib)
#define LOADFUNCTION(lib, func) GetProcAddress(lib, func)
#define UNLOADLIBRARY(lib) FreeLibrary(lib)
#endif

using namespace dasa::gliese::scanner::twain;

bool DSM::load() {
    if (state() != State::Unloaded) {
        LOG_F(1, "Tried to load DSM when it is already loaded");
        return true;
    }

    // In OSX, TWAIN is a framework, statically linked
#ifdef __APPLE__
    entry = &DSM_Entry;
#else
    auto library = LOADLIBRARY(dsm_lib_path.c_str());

    if (library == nullptr) {
        LOG_S(FATAL) << "Failed to open TWAIN library at path [" << dsm_lib_path << "]";
        return false;
    }

    entry = reinterpret_cast<DSMENTRYPROC>(LOADFUNCTION(library, "DSM_Entry"));
    LOG_F(INFO, "DSM module allocated");

#ifdef TWH_CMP_GNU
    dsm_module = DSM::module_ptr_t(library, module_releaser());

    auto err = dlerror();
    if (err) {
        ABORT_S() << "Failed to get TWAIN DSM_Entry: " << err;
        return false;
    }
#else
    dsm_module = DSM::module_ptr_t(&library, module_releaser());

    if (entry == nullptr) {
        ABORT_S() << "Failed to get TWAIN DSM_Entry";
        return false;
    }
#endif
#endif

    state(State::Disconnected);
    LOG_F(INFO, "DSM library loaded");
    return true;
}

void DSM::unload() {
    if (state() == State::Unloaded) {
        LOG_F(1, "Trying to unloading DSM library when it is not loaded");
        return;
    }
    if (state() == State::Ready) {
        LOG_F(ERROR, "Trying to unload DSM library when it is in use");
        return;
    }

    dsm_module = nullptr;
    entry = nullptr;
    state(State::Unloaded);
    LOG_F(INFO, "DSM library unloaded");
}

bool DSM::open(pTW_IDENTITY identity, TW_MEMREF parent) {
    if (state() == State::Unloaded) {
        LOG_F(ERROR, "Trying to open DSM connection when library is not loaded");
        return false;
    }
    if (state() == State::Ready) {
        LOG_F(1, "Trying to open DSM connection when it is already open");
        return true;
    }

    auto result = (*this)(identity, nullptr, DG_CONTROL, DAT_PARENT, MSG_OPENDSM, parent);
    if (result != TWRC_SUCCESS) {
        return false;
    }

    std::fill_n((char*)&entrypoint, sizeof(TW_ENTRYPOINT), 0);
    entrypoint.Size = sizeof(TW_ENTRYPOINT);

    if ((identity->SupportedGroups & (uint32_t)DF_DSM2) == DF_DSM2) {
        result = (*this)(identity, nullptr, DG_CONTROL, DAT_ENTRYPOINT, MSG_GET, reinterpret_cast<TW_MEMREF>(&entrypoint));
        if (result != TWRC_SUCCESS) {
            return false;
        }
    }

    LOG_F(INFO, "DSM connection open");
    state(State::Ready);

    return true;
}

void DSM::close(pTW_IDENTITY identity, TW_MEMREF parent) {
    if (state() == State::Unloaded) {
        LOG_F(ERROR, "Trying to close DSM connection when library is not loaded");
        return;
    }
    if (state() == State::Disconnected) {
        LOG_F(1, "Trying to close DSM library when it is already open");
        return;
    }

    if ((*this)(identity, nullptr, DG_CONTROL, DAT_PARENT, MSG_CLOSEDSM, parent) != TWRC_SUCCESS) {
        LOG_F(ERROR, "Failed to close the DSM connection");
        return;
    }

    LOG_F(INFO, "DSM connection closed");
    state(State::Disconnected);
}

TW_HANDLE DSM::alloc(TW_UINT32 size) {
    if (entrypoint.DSM_MemAllocate) {
        return entrypoint.DSM_MemAllocate(size);
    }

#ifdef TWH_CMP_MSC
    return ::GlobalAlloc(GPTR, size);
#else
    return reinterpret_cast<TW_HANDLE>(malloc(size));
#endif
}

void DSM::free(TW_HANDLE memory)
{
    if(entrypoint.DSM_MemFree)
    {
        return entrypoint.DSM_MemFree(memory);
    }

#ifdef TWH_CMP_MSC
    ::GlobalFree(memory);
#else
    ::free(memory);
#endif
}


TW_MEMREF DSM::lock(TW_HANDLE memory) {
    if (entrypoint.DSM_MemLock) {
        return entrypoint.DSM_MemLock(memory);
    }

#ifdef TWH_CMP_MSC
    return (TW_MEMREF)::GlobalLock(memory);
#else
    return reinterpret_cast<TW_MEMREF>(memory);
#endif
}

void DSM::unlock(TW_HANDLE memory)
{
    if(entrypoint.DSM_MemUnlock)
    {
        return entrypoint.DSM_MemUnlock(memory);
    }

#ifdef TWH_CMP_MSC
    ::GlobalUnlock(memory);
#endif
}

TW_UINT16 DSM::operator()(pTW_IDENTITY pOrigin, pTW_IDENTITY pDest, TW_UINT32 DG, TW_UINT16 DAT, TW_UINT16 MSG, TW_MEMREF pData) {
    if (!entry) {
        return TWRC_FAILURE;
    }

    LOG_S(9) << DG << " / " << DAT << " / " << MSG;
    return entry(pOrigin, pDest, DG, DAT, MSG, pData);
}

void DSM::module_releaser::operator()(DSM::module_t *module) const {
    LOG_F(INFO, "DSM module released");
#ifdef TWH_CMP_MSC
    UNLOADLIBRARY(*module);
#else
    UNLOADLIBRARY(module);
#endif
}
