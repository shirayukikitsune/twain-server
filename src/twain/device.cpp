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

#include "device.hpp"
#include "../twain.hpp"

using namespace dasa::gliese::scanner::twain;

bool Device::operator==(TW_ID other) {
    return identity.Id == other;
}

bool Device::operator==(const TW_IDENTITY &other) {
    return identity.Id == other.Id;
}

bool Device::isOnline() {
    // We need to be on state 3 for the DS to be opened
    if (twain->getState() == 2) {
        twain->openDSM();
    }

    if (twain->getState() != 3 || (twain->getDataSource() != nullptr && twain->getDataSource()->Id != identity.Id)) {
        return false;
    }

    bool closeAfter = false;
    if (twain->getState() == 3) {
        twain->loadDataSource(*this);
        closeAfter = true;
    }

    TW_CAPABILITY cap { CAP_DEVICEONLINE, 0, nullptr };
    auto rc = twain->getCapability(cap, MSG_GETCURRENT);
    bool status = false;

    if (rc != TWRC_SUCCESS) {
        goto finish;
    }

    status = static_cast<TW_BOOL>(((pTW_ONEVALUE)cap.hContainer)->Item) != 0;

finish:
    if (closeAfter) {
        twain->closeDS();
    }

    return status;
}

nlohmann::json Device::toJson() {
    auto json = deviceToJson(identity);
    //json["online"] = isOnline();
    return json;
}

nlohmann::json deviceToJson(TW_IDENTITY device) {
    nlohmann::json deviceJson;
#ifdef __APPLE__
    deviceJson["id"] = reinterpret_cast<unsigned long>(device.Id);
#else
    deviceJson["id"] = static_cast<unsigned long>(device.Id);
#endif
    deviceJson["productName"] = reinterpret_cast<char*>(device.ProductName);
    deviceJson["manufacturer"] = reinterpret_cast<char*>(device.Manufacturer);
    deviceJson["productFamily"] = reinterpret_cast<char*>(device.ProductFamily);
    return deviceJson;
}

std::ostream &operator<<(std::ostream& os, const Device& device) {
    os << (TW_IDENTITY)device;
    return os;
}
