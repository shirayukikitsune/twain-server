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

#include <system_error>

namespace dasa::gliese::scanner::twain {
    enum class error_code {
        generic_failure = 1,
        cancelled,
        invalid_state
    };

    namespace detail {
        class twain_category : public std::error_category {
        public:
            const char *name() const noexcept final {
                return "TWAIN";
            }

            std::string message(int c) const final;
        };
    }

    extern inline const detail::twain_category& twain_category() {
        static detail::twain_category category;
        return category;
    }

    inline std::error_code make_error_code(error_code ec) {
        return { static_cast<int>(ec), twain_category() };
    }
}

namespace std {
    template<>
    struct is_error_code_enum<dasa::gliese::scanner::twain::error_code> : std::true_type {};
}

namespace dasa::gliese::scanner::twain {

    class twain_error : public std::system_error {
    public:
        explicit twain_error(error_code e) : std::system_error(make_error_code(e)) {}
    };

}