#pragma once

#include <system_error>

namespace dasa::gliese::scanner::twain {
    enum class error_code {
        success = 0,
        generic_failure,
        invalid_state
    };

    namespace detail {
        class twain_category : public std::error_category {
        public:
            const char *name() const noexcept final {
                return "TWAIN";
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

    extern inline const detail::twain_category& twain_category() {
        static detail::twain_category category;
        return category;
    }
}

namespace std {
    template<>
    struct is_error_condition_enum<dasa::gliese::scanner::twain::error_code> : std::true_type {};

    inline error_code make_error_code(dasa::gliese::scanner::twain::error_code error_code) {
        return { static_cast<int>(error_code), dasa::gliese::scanner::twain::twain_category() };
    }
}

namespace dasa::gliese::scanner::twain {

    class twain_error : public std::system_error {
    public:
        explicit twain_error(error_code e) : std::system_error(std::make_error_code(e)) {}
    };

}