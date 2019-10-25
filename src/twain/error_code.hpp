#pragma once

#include <boost/system/error_code.hpp>

namespace dasa::gliese::scanner::twain {
    enum class error_code {
        success = 0,
        generic_failure,
        invalid_state
    };

    namespace detail {
        class error_code_category : public boost::system::error_category {
        public:
            const char* name() const noexcept final {
                return "TWAIN Error";
            }
            std::string message(int c) const final {
                switch (static_cast<error_code>(c)) {
                case error_code::success:
                    return "no error";
                case error_code::generic_failure:
                    return "generic failure";
                case error_code::invalid_state:
                    return "invalid state";
                default:
                    return "unknown error";
                }
            }
        };
    }

    extern inline const detail::error_code_category& category() {
        static detail::error_code_category category;
        return category;
    }
}

namespace boost::system {
    template<>
    struct is_error_code_enum<dasa::gliese::scanner::twain::error_code> : std::true_type {};
}

inline boost::system::error_code make_error_code(dasa::gliese::scanner::twain::error_code error_code) {
    return { static_cast<int>(error_code), dasa::gliese::scanner::twain::category() };
}
