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

#include "application.hpp"

#include "twain.hpp"

#ifdef ENABLE_IMAGE_CONVERSION
#include "twain/transfer/conversion_transfer.hpp"
#endif
#include "twain/transfer/memory_transfer.hpp"
#include "twain/transfer/native_transfer.hpp"

#include <cstring>
#include <list>
#include <loguru.hpp>

using dasa::gliese::scanner::Twain;
extern dasa::gliese::scanner::Application *application;

Twain::Twain(boost::asio::io_context& context)
        : context(context) {

}

void Twain::fillIdentity() {
#ifdef __APPLE__
    identity.Id = nullptr;
#else
    identity.Id = 0;
#endif
    identity.Version.Country = TWCY_BRAZIL;
    strncpy(reinterpret_cast<char*>(identity.Version.Info), "1.0.0", 32);
    identity.Version.Language = TWLG_PORTUGUESE_BRAZIL;
    identity.Version.MajorNum = 1;
    identity.Version.MinorNum = 0;
    identity.ProtocolMajor = 2;
    identity.ProtocolMinor = 4;
    identity.SupportedGroups = DF_APP2 | DG_CONTROL | DG_IMAGE;
    strncpy(reinterpret_cast<char*>(identity.Manufacturer), "Diagnosticos da America SA", 32);
    strncpy(reinterpret_cast<char*>(identity.ProductFamily), "Gliese", 32);
    strncpy(reinterpret_cast<char*>(identity.ProductName), "Scanner Integration", 32);
}

void twainListen();

void Twain::loadDSM(const char *path) {
    if (state > 1) {
        LOG_S(WARNING) << "Trying to load DSM library when it is already loaded";
        return;
    }

    if (path != nullptr) {
        DSM.path(path);
    }

    if (!DSM.load()) {
        LOG_S(ERROR) << "Failed to load DSM library";
        return;
    }

    setState(2);

    twainListen();
}

void twainListen() {
    if (application->getTwain().getState() == 1) {
        return;
    }

    boost::asio::post(application->getTwainIoContext(), [] {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        twainListen();
    });
}

void Twain::unloadDSM() {
    if (state > 2) {
        LOG_S(ERROR) << "Trying to unload DSM when it is open";
        return;
    }

    DSM.unload();
    setState(1);
}

void Twain::openDSM() {
    if (state > 3) {
        LOG_S(WARNING) << "Trying to open DSM connection when a connection is already open";
        return;
    }
    if (state == 1) {
        ABORT_S() << "Trying to open DSM connection when DSM library is not loaded";
        return;
    }

    auto parent = application->getParentWindow();
    if (!DSM.open(getIdentity(), reinterpret_cast<TW_MEMREF>(&parent))) {
        LOG_S(ERROR) << "Failed to open DSM connection";
        return;
    }

    setState(3);
}

void Twain::closeDSM() {
    if (state < 3) {
        LOG_S(WARNING) << "Trying to close the DSM when it is not open";
        return;
    }

    auto parent = application->getParentWindow();
    DSM.close(getIdentity(), reinterpret_cast<TW_MEMREF>(&parent));
    setState(2);
}

void Twain::setState(int newState)
{
    LOG_F(INFO, "Changing state from %d to %d", state, newState);
    state = newState;
}

std::list<dasa::gliese::scanner::twain::Device> Twain::listSources() {
    LOG_SCOPE_FUNCTION(INFO);
    std::list<twain::Device> sources;

    if (state < 3) {
        LOG_S(ERROR) << "Trying to list sources when DSM is not active";
        throw twain::twain_error(twain::error_code::invalid_state);
    }

    TW_IDENTITY current;
    memset(&current, 0, sizeof(TW_IDENTITY));
    auto rc = DSM(&identity, nullptr, DG_CONTROL, DAT_IDENTITY, MSG_GETFIRST, reinterpret_cast<TW_MEMREF>(&current));
    if (rc != TWRC_SUCCESS && rc != TWRC_ENDOFLIST) {
        LOG_S(ERROR) << "Failed to get the list of available sources";
        throw twain::twain_error(twain::error_code::generic_failure);
    }

    do {
        sources.emplace_back(this, current);
        memset(&current, 0, sizeof(TW_IDENTITY));
    } while (DSM(&identity, nullptr, DG_CONTROL, DAT_IDENTITY, MSG_GETNEXT, reinterpret_cast<TW_MEMREF>(&current)) == TWRC_SUCCESS);

    LOG_S(INFO) << "Found " << sources.size() << " DS";

    return sources;
}

std::list<dasa::gliese::scanner::twain::Device> Twain::listSources(std::error_code& ec) noexcept {
    try {
        return listSources();
    }
    catch (std::system_error& e) {
        ec = e.code();
        return std::list<twain::Device>();
    }
}

TW_IDENTITY Twain::getDefaultDataSource() {
    TW_IDENTITY current;
    memset(&current, 0, sizeof(TW_IDENTITY));

    if (state < 3) {
        LOG_S(ERROR) << "Trying to get default source when DSM is not active";
        return current;
    }

    auto rc = DSM(getIdentity(), nullptr, DG_CONTROL, DAT_IDENTITY, MSG_GETDEFAULT, reinterpret_cast<TW_MEMREF>(&current));
    if (rc != TWRC_SUCCESS) {
        getStatus();
    }
    return current;
}

TW_STATUSUTF8 Twain::getStatus() {
    TW_STATUSUTF8 twStatus;
    memset(&twStatus, 0, sizeof(TW_STATUSUTF8));

    if (state < 3) {
        LOG_S(ERROR) << "Trying to get status when DSM is not active";
        return twStatus;
    }

    (*this)(DG_CONTROL, DAT_STATUSUTF8, MSG_GET, reinterpret_cast<TW_MEMREF>(&twStatus));
    return twStatus;
}

static TW_UINT16 DSMCallback(pTW_IDENTITY origin, pTW_IDENTITY /*dest*/, TW_UINT32 /*dg*/, TW_UINT16 /*dat*/, TW_UINT16 message, TW_MEMREF /*data*/) {
    auto& twain = application->getTwain();
    if (origin == nullptr || origin->Id != (dasa::gliese::scanner::twain::Device::TW_ID)(*twain.getDataSource())) {
        return TWRC_FAILURE;
    }

    switch (message) {
        case MSG_XFERREADY:
            twain.setState(6);
        case MSG_CLOSEDSREQ:
        case MSG_CLOSEDSOK:
        case MSG_NULL:
            LOG_S(INFO) << "Got message from DSM: " << message;
            break;
        default:
            LOG_S(INFO) << "Got unknown message from DSM: " << message;
            return TWRC_FAILURE;
    }

    return TWRC_SUCCESS;
}

bool Twain::loadDataSource(dasa::gliese::scanner::twain::Device::TW_ID id) {
    if (state < 3) {
        LOG_S(ERROR) << "Trying to load DS when DSM is not active";
        return false;
    }
    if (state > 3) {
        LOG_S(WARNING) << "A source is already open";
        return false;
    }

    currentDS = make_device(id);

    if (!currentDS) {
#ifdef __APPLE__
        LOG_S(ERROR) << "Could not find DS with id " << reinterpret_cast<uintptr_t>(id);
#else
        LOG_S(ERROR) << "Could not find DS with id " << id;
#endif
        return false;
    }

    return loadDataSource(*currentDS);
}

bool Twain::loadDataSource(dasa::gliese::scanner::twain::Device &device) {
    if (state < 3) {
        LOG_S(ERROR) << "Trying to load DS when DSM is not active";
        return false;
    }
    if (state > 3) {
        LOG_S(WARNING) << "A source is already open";
        return false;
    }

    if (!currentDS || currentDS->getIdentity()->Id != device.getIdentity()->Id) {
        currentDS = std::make_unique<twain::Device>(this, *device.getIdentity());
    }

    TW_CALLBACK callback;
    memset(&callback, 0, sizeof(TW_CALLBACK));
    callback.CallBackProc = (TW_MEMREF)&DSMCallback;
#ifdef __APPLE__
    callback.RefCon = nullptr;
#else
    callback.RefCon = 0;
#endif
    useCallbacks = false;

    auto resultCode = DSM(getIdentity(), nullptr, DG_CONTROL, DAT_IDENTITY, MSG_OPENDS, reinterpret_cast<TW_MEMREF>(currentDS->getIdentity()));

    if (resultCode != TWRC_SUCCESS) {
        LOG_S(ERROR) << "Failed to open DataSource";
        currentDS.reset(nullptr);
        return false;
    }

    setState(4);

#if defined(WIN32) || defined(WIN64) || defined (_WINDOWS)
    if ((getIdentity()->SupportedGroups & DF_DSM2) && (currentDS->getIdentity()->SupportedGroups & DF_DSM2)) {
        resultCode = (*this)(DG_CONTROL, DAT_CALLBACK, MSG_REGISTER_CALLBACK, &callback);
        if (resultCode != TWRC_SUCCESS) {
            LOG_S(ERROR) << "Failed to register callback: " << resultCode;
            return false;
        } else {
            useCallbacks = true;
        }
    } else {
        LOG_S(WARNING) << "MSG_REGISTER_CALLBACK not supported";
    }
#else
    resultCode = (*this)(DG_CONTROL, DAT_CALLBACK, MSG_REGISTER_CALLBACK, reinterpret_cast<TW_MEMREF>(&callback));
    if (resultCode != TWRC_SUCCESS) {
        LOG_S(ERROR) << "Failed to register callback: " << resultCode;
        return false;
    } else {
        useCallbacks = true;
    }
#endif

    return true;
}

void Twain::enableDataSource(TW_HANDLE handle, bool showUI) {
    if (getState() == 2) {
        openDSM();
    }

    boost::asio::post(application->getTwainIoContext(), [this, &handle, showUI] {
        if (getState() < 4) {
            LOG_S(ERROR) << "Trying to enable DS but it was not opened";
            return;
        }

        memset(&ui, 0, sizeof(TW_USERINTERFACE));

        ui.ShowUI = showUI;
        ui.ModalUI = 1;
        ui.hParent = handle;

        auto resultCode = (*this)(DG_CONTROL, DAT_USERINTERFACE, MSG_ENABLEDS, reinterpret_cast<TW_MEMREF>(&ui));
        if (resultCode != TWRC_SUCCESS && resultCode != TWRC_CHECKSTATUS) {
            LOG_S(ERROR) << "Failed to enable DS: " << resultCode;
            return;
        }

        if (getState() == 4) {
            setState(5);
        }
    });
}

bool Twain::closeDS() {
    if (state < 4) {
        LOG_S(ERROR) << "Trying to close DS but it was not opened";
        return false;
    }

    auto resultCode = DSM(getIdentity(), nullptr, DG_CONTROL, DAT_IDENTITY, MSG_CLOSEDS, reinterpret_cast<TW_MEMREF>(currentDS.get()));

    if (resultCode != TWRC_SUCCESS) {
        auto status = getStatus();
        LOG_S(ERROR) << "Failed to close DS: RC " << resultCode << "; CC " << status.Status.ConditionCode;
    }

    setState(3);
    LOG_S(INFO) << "DS closed";
    currentDS = nullptr;

    return true;
}

void Twain::disableDS() {
    if (state < 5) {
        LOG_S(ERROR) << "Trying to disable DS but it was not enabled";
        return;
    }

    auto resultCode = (*this)(DG_CONTROL, DAT_USERINTERFACE, MSG_DISABLEDS, reinterpret_cast<TW_MEMREF>(&ui));

    if (resultCode != TWRC_SUCCESS) {
        auto status = getStatus();
        LOG_S(ERROR) << "Failed to disable DS: RC " << resultCode << "; CC " << status.Status.ConditionCode;
    }

    setState(4);
    LOG_S(INFO) << "DS disabled";
}

TW_UINT16 Twain::setCapability(TW_UINT16 capability, int value, TW_UINT16 type) {
    TW_UINT16 returnCode = TWRC_FAILURE;

    TW_CAPABILITY cap;
    cap.Cap = capability;
    cap.ConType = TWON_ONEVALUE;
    cap.hContainer = DSM.alloc(sizeof(TW_ONEVALUE));
    if (cap.hContainer == nullptr) {
        ABORT_S() << "Error allocating memory";
        return returnCode;
    }

    auto pValue = (pTW_ONEVALUE)DSM.lock(cap.hContainer);
    pValue->ItemType = type;

    switch (type) {
        case TWTY_INT8:
            *(TW_INT8*)&pValue->Item = (TW_INT8)value;
            break;

        case TWTY_INT16:
            *(TW_INT16*)&pValue->Item = (TW_INT16)value;
            break;

        case TWTY_INT32:
            *(TW_INT32*)&pValue->Item = (TW_INT32)value;
            break;

        case TWTY_UINT8:
            *(TW_UINT8*)&pValue->Item = (TW_UINT8)value;
            break;

        case TWTY_UINT16:
            *(TW_UINT16*)&pValue->Item = (TW_UINT16)value;
            break;

        case TWTY_UINT32:
            memcpy(&pValue->Item,&value,sizeof(TW_UINT32));
            break;

        case TWTY_BOOL:
            memcpy(&pValue->Item,&value,sizeof(TW_BOOL));
            break;

        default:
            goto cleanup;
    }

    returnCode = (*this)(DG_CONTROL, DAT_CAPABILITY, MSG_SET, reinterpret_cast<TW_MEMREF>(&cap));
    if (returnCode == TWRC_FAILURE) {
        LOG_S(ERROR) << "Failed to set capability";
    }

cleanup:
    DSM.unlock(cap.hContainer);
    DSM.free(cap.hContainer);

    return returnCode;
}

typedef struct {
	TW_UINT16  ItemType;
#ifdef __APPLE__
	TW_UINT16  Dummy;
#endif
	TW_FIX32   Item;
} TW_ONEVALUE_FIX32, FAR* pTW_ONEVALUE_FIX32;

TW_UINT16 Twain::setCapability(TW_UINT16 Cap, const TW_FIX32* _pValue) {
	TW_INT16        twrc = TWRC_FAILURE;
	TW_CAPABILITY   cap;

	cap.Cap = Cap;
	cap.ConType = TWON_ONEVALUE;
	cap.hContainer = DSM.alloc(sizeof(TW_ONEVALUE_FIX32));
	if (nullptr == cap.hContainer)
	{
		LOG_S(ERROR) << "Error allocating memory";
		return twrc;
	}

	auto pVal = reinterpret_cast<pTW_ONEVALUE_FIX32>(DSM.lock(cap.hContainer));

	pVal->ItemType = TWTY_FIX32;
	pVal->Item = *_pValue;

	// capability structure is set, make the call to the source now
	twrc = (*this)(DG_CONTROL, DAT_CAPABILITY, MSG_SET, reinterpret_cast<TW_MEMREF>(&cap));
	if (TWRC_FAILURE == twrc)
	{
		LOG_S(ERROR) << "Could not set capability";
	}

	DSM.unlock(cap.hContainer);
	DSM.free(cap.hContainer);

	return twrc;
}

std::weak_ptr<dasa::gliese::scanner::twain::Transfer> Twain::startScan(const std::string &outputMime) {
    auto ptr = std::weak_ptr<twain::Transfer>();
    if (state != 6) {
        LOG_S(ERROR) << "Cannot start scanning unless if scanner is ready";
        return ptr;
    }

    TW_CAPABILITY xferCap = {ICAP_XFERMECH, 0, nullptr };
    auto rc = getCapability(xferCap);
    TW_UINT32 mech;

    if (rc == TWRC_FAILURE) {
        return ptr;
    }
    if (!getCurrent(&xferCap, mech)) {
        LOG_S(ERROR) << "Failed to get current ICAP_XFERMECH value";
        return ptr;
    }

    switch (mech)
    {
    case TWSX_MEMORY:
        activeTransfer = std::make_shared<dasa::gliese::scanner::twain::MemoryTransfer>(this, outputMime);
        break;
    case TWSX_NATIVE:
        activeTransfer = std::make_shared<dasa::gliese::scanner::twain::NativeTransfer>(this, outputMime);
        break;
    default:
        LOG_S(ERROR) << "Unsupported ICAP_XFERMECH " << mech;
        break;
    }

#ifdef ENABLE_IMAGE_CONVERSION
    auto conversion_transfer = std::make_shared<twain::conversion_transfer>(this, outputMime);
    conversion_transfer->set_transfer(activeTransfer);
    activeTransfer = conversion_transfer;
#endif

    ptr = activeTransfer;
	return ptr;
}

TW_INT16 Twain::getCapability(TW_CAPABILITY& _cap, TW_UINT16 _msg) {
    if(_msg != MSG_GET && _msg != MSG_GETCURRENT && _msg != MSG_GETDEFAULT && _msg != MSG_RESET) {
        LOG_S(ERROR) << "Bad message";
        return TWRC_FAILURE;
    }

    if(state < 4) {
        LOG_S(ERROR) << "You need to open a data source first.";
        return TWRC_FAILURE;
    }

    // Check if this capability structure has memory already alloc'd.
    // If it does, free that memory before the call else we'll have a memory
    // leak because the source allocates memory during a MSG_GET.
    if (_cap.hContainer)
    {
        DSM.free(_cap.hContainer);
        _cap.hContainer = nullptr;
    }

    _cap.ConType = TWON_DONTCARE16;

    // capability structure is set, make the call to the source now
    TW_UINT16 twrc = (*this)(DG_CONTROL, DAT_CAPABILITY, _msg, reinterpret_cast<TW_MEMREF>(&_cap));

    return twrc;
}

std::unique_ptr<dasa::gliese::scanner::twain::Device> Twain::make_device(dasa::gliese::scanner::twain::Device::TW_ID id)
{
    std::error_code ec;
    auto sources = listSources(ec);
    if (ec) {
        return nullptr;
    }

    for (auto& source : sources) {
        if (source == id) {
            return std::make_unique<twain::Device>(this, *source.getIdentity());
        }
    }

    return nullptr;
}

bool Twain::getCurrent(TW_CAPABILITY *pCap, TW_UINT32& val)
{
    if (!pCap->hContainer) {
        return false;
    }

    bool ret = false;
    if (pCap->ConType == TWON_ENUMERATION) {
        auto pCapPT = (pTW_ENUMERATION)DSM.lock(pCap->hContainer);
        switch(pCapPT->ItemType) {
            case TWTY_INT32:
                val = (TW_INT32)((pTW_INT32)(&pCapPT->ItemList))[pCapPT->CurrentIndex];
                ret = true;
                break;

            case TWTY_UINT32:
                val = (TW_INT32)((pTW_UINT32)(&pCapPT->ItemList))[pCapPT->CurrentIndex];
                ret = true;
                break;

            case TWTY_INT16:
                val = (TW_INT32)((pTW_INT16)(&pCapPT->ItemList))[pCapPT->CurrentIndex];
                ret = true;
                break;

            case TWTY_UINT16:
                val = (TW_INT32)((pTW_UINT16)(&pCapPT->ItemList))[pCapPT->CurrentIndex];
                ret = true;
                break;

            case TWTY_INT8:
                val = (TW_INT32)((pTW_INT8)(&pCapPT->ItemList))[pCapPT->CurrentIndex];
                ret = true;
                break;

            case TWTY_UINT8:
                val = (TW_INT32)((pTW_UINT8)(&pCapPT->ItemList))[pCapPT->CurrentIndex];
                ret = true;
                break;

            case TWTY_BOOL:
                val = (TW_INT32)((pTW_BOOL)(&pCapPT->ItemList))[pCapPT->CurrentIndex];
                ret = true;
                break;
        }
        DSM.unlock(pCap->hContainer);
    }
    else if(TWON_ONEVALUE == pCap->ConType) {
        auto pCapPT = (pTW_ONEVALUE)DSM.lock(pCap->hContainer);
        if(pCapPT->ItemType < TWTY_FIX32) {
            val = pCapPT->Item;
            ret = true;
        }
        DSM.unlock(pCap->hContainer);
    }
    else if(TWON_RANGE == pCap->ConType) {
        auto pCapPT = (pTW_RANGE)DSM.lock(pCap->hContainer);
        if(pCapPT->ItemType < TWTY_FIX32) {
            val = pCapPT->CurrentValue;
            ret = true;
        }
        DSM.unlock(pCap->hContainer);
    }

    return ret;
}

void Twain::shutdown() noexcept {
    LOG_SCOPE_FUNCTION(INFO);

    if (getState() >= 6) {
        LOG_F(INFO, "Canceling transfer");
        activeTransfer->clearPending();
        this->endTransfer();
    }
    if (getState() == 5) {
        LOG_F(INFO, "Disabling DS");
        this->disableDS();
    }
    if (getState() == 4) {
        LOG_F(INFO, "Closing DS");
        this->closeDS();
    }
    if (getState() == 3) {
        LOG_F(INFO, "Closing DSM");
        this->closeDSM();
    }
    if (getState() == 2) {
        LOG_F(INFO, "Unloading DSM");
        this->unloadDSM();
    }
}

void Twain::reset() {
    shutdown();

    if (state != 1) {
        LOG_S(ERROR) << "Failed to reset TWAIN state";
        return;
    }

    loadDSM(nullptr);
    openDSM();
}

std::tuple<std::list<uint32_t>, std::list<uint32_t>> dasa::gliese::scanner::Twain::getDeviceDPIs(twain::Device::TW_ID device)
{
    if (state < 3 || (state > 3 && device != currentDS->getIdentity()->Id)) {
        LOG_S(ERROR) << "Invalid DSM state";
        return std::make_tuple(std::list<uint32_t>(), std::list<uint32_t>());
    }

    if (currentDS && currentDS->getIdentity()->Id == device) {
        return currentDS->getResolutions();
    }

    auto d = make_device(device);
    auto response = std::make_tuple(std::list<uint32_t>(), std::list<uint32_t>());

    if (d) {
        response = d->getResolutions();
    }

    return response;
}

void Twain::endTransfer() {
    if (!activeTransfer) {
        LOG_S(INFO) << "Trying to end a transfer when there is none active";
        return;
    }

    activeTransfer->clearPending();
    activeTransfer->end();
    activeTransfer = nullptr;
}

std::ostream& operator<<(std::ostream& os, const TW_IDENTITY& identity) {
#ifdef __APPLE__
    os << "Device " << reinterpret_cast<std::ptrdiff_t>(identity.Id);
    if (identity.Id != nullptr) {
#else
    os << "Device " << static_cast<std::ptrdiff_t>(identity.Id);
    if (identity.Id != 0) {
#endif
        os << ": " << identity.Manufacturer << ", " << identity.ProductName << ", " << identity.ProductFamily;
    } else {
        os << " has no data";
    }
    return os;
}
