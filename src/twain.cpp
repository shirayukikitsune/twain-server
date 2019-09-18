#pragma ide diagnostic ignored "hicpp-signed-bitwise"
#include "twain.hpp"

#include "exception/dsm_exception.hpp"

#include <cstring>
#include <list>

using dasa::gliese::scanner::Twain;
using namespace dasa::gliese::scanner::exception;

Twain::~Twain() {
    closeDSM();
}

void Twain::fillIdentity() {
    identity.Id = 0;
    identity.Version.Country = TWCY_BRAZIL;
    strncpy(identity.Version.Info, "1.0.0", 32);
    identity.Version.Language = TWLG_PORTUGUESE_BRAZIL;
    identity.Version.MajorNum = 1;
    identity.Version.MinorNum = 0;
    identity.ProtocolMajor = 2;
    identity.ProtocolMinor = 4;
    identity.SupportedGroups = DF_APP2 | DG_CONTROL | DG_IMAGE;
    strncpy(identity.Manufacturer, "Diagnosticos da America SA", 32);
    strncpy(identity.ProductFamily, "Gliese", 32);
    strncpy(identity.ProductName, "Scanner Integration", 32);
}

void Twain::loadDSM(const char *path) {
    DSM.load(path);
    entry = DSM.get<TW_UINT16(pTW_IDENTITY, pTW_IDENTITY, TW_UINT32, TW_UINT16, TW_UINT16, TW_MEMREF)>("DSM_Entry");
}

void Twain::openDSM() {
    auto result = entry(getIdentity(), nullptr, DG_CONTROL, DAT_PARENT, MSG_OPENDSM, nullptr);
    if (result != TWRC_SUCCESS) {
        throw DSMOpenException(result);
    }

    if ((identity.SupportedGroups & DF_DSM2) == DF_DSM2) {
        entrypoint.Size = sizeof(TW_ENTRYPOINT);
        result = entry(getIdentity(), nullptr, DG_CONTROL, DAT_ENTRYPOINT, MSG_GET, &entrypoint);
        if (result != TWRC_SUCCESS) {
            throw DSMOpenException(result);
        }
    }

    isOpen = true;
}

void Twain::closeDSM() {
    if (!isOpen) {
        return;
    }

    entry(getIdentity(), nullptr, DG_CONTROL, DAT_PARENT, MSG_CLOSEDSM, nullptr);
}

std::list<TW_IDENTITY> Twain::listSources() {
    std::list<TW_IDENTITY> sources;
    TW_IDENTITY current;
    memset(&current, 0, sizeof(TW_IDENTITY));
    if (entry(&identity, 0, DG_CONTROL, DAT_IDENTITY, MSG_GETFIRST, (TW_MEMREF)&current) != TWRC_SUCCESS) {
        return sources;
    }

    do {
        sources.push_back(current);
        memset(&current, 0, sizeof(TW_IDENTITY));
    } while (entry(&identity, 0, DG_CONTROL, DAT_IDENTITY, MSG_GETNEXT, (TW_MEMREF)&current) == TWRC_SUCCESS);

    return sources;
}

TW_IDENTITY Twain::getDefaultDataSource() {
    TW_IDENTITY current;
    memset(&current, 0, sizeof(TW_IDENTITY));
    auto rc = entry(getIdentity(), 0, DG_CONTROL, DAT_IDENTITY, MSG_GETDEFAULT, (TW_MEMREF)&current);
    TW_STATUS s;
    if (rc != TWRC_SUCCESS) {
        s = getStatus(rc);
    }
    TWCC_BADCAP;
    return current;
}

TW_STATUS Twain::getStatus(TW_UINT16 rc) {
    TW_STATUS twStatus;
    entry(getIdentity(), 0, DG_CONTROL, DAT_STATUS, MSG_GET, &twStatus);
    return twStatus;
}
