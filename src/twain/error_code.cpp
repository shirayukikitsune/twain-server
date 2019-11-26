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

#include "error_code.hpp"

namespace dasa::gliese::scanner::twain::detail {
    std::string twain_category::message(int c) const {
        switch (static_cast<error_code>(c)) {
        case error_code::generic_failure:
            return "generic failure";
        case error_code::cancelled:
            return "cancelled";
        case error_code::invalid_state:
            return "invalid state";
        default:
            return "unknown error";
        }
    }
}
