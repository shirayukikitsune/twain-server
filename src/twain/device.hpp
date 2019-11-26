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

#include <list>
#include <nlohmann/json.hpp>
#include <ostream>
#include <tuple>
#include "../external/twain.h"

namespace dasa::gliese::scanner {
    class Twain;
}

namespace dasa::gliese::scanner::twain {
    class Device {
    public:
        // TWAIN in MacOS uses an uintptr_t for the ID, instead of an integer. We typedef that to avoid several #ifdefs
        /// The type of a device identifier
        typedef decltype(TW_IDENTITY::Id) TW_ID;

        Device(Twain *twain, TW_IDENTITY identity) : identity(identity) {
            this->twain = twain;
        }

        explicit operator TW_IDENTITY() {
            return identity;
        }

        explicit operator TW_IDENTITY() const {
            return identity;
        }

        explicit operator TW_ID() {
            return identity.Id;
        }

        bool operator==(TW_ID other);
        bool operator==(const TW_IDENTITY &other);

        pTW_IDENTITY getIdentity() {
            return &identity;
        }

        std::tuple<std::list<uint32_t>, std::list<uint32_t>> getResolutions();

        bool isOnline();

        nlohmann::json toJson();

    private:
        TW_IDENTITY identity;
        Twain *twain;

        bool getResolution(TW_UINT16 cap, std::list<uint32_t> &resolutions);
    };
}

nlohmann::json deviceToJson(TW_IDENTITY device);
std::ostream& operator<<(std::ostream &os, const dasa::gliese::scanner::twain::Device& device);
