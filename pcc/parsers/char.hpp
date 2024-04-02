#pragma once
#include <pcc/parsers/take.hpp>

namespace pcc {

/// @brief Take spaces from source string
inline auto spaces0() {
    return take_while([](unsigned char ch) { return std::isspace(ch); });
}

/// @brief Take at least N spaces from source string
/// @param at_least<N> where N is number of spaces to take
/// @return parser result
template <std::size_t N>
auto spaces(detail::at_least<N>) {
    return parser{[](const auto &src) {
        return spaces0()(src).and_then([](const auto &res) {
            if (res.value.size() < N)
                return not_parsed<decltype(res.value),
                                  typename decltype(res.rest)::value_type>(
                    "Do not enough symbols");

            return parsed(res.value, res.rest);
        });
    }};
}

/// @brief Take at least one space from source string
inline auto spaces1() { return spaces(at_least<1>()); }

/// @brief Take alphanumeric symbols from source string
inline auto alphanumeric0() {
    return take_while([](unsigned char ch) { return std::isalnum(ch); });
}

/// @brief Take at least N alphanumeric symbols from source string
/// @param at_least<N> where N is number of symbols to take
/// @return parser result
template <std::size_t N>
auto alphanumeric(detail::at_least<N>) {
    return parser{[](const auto &src) {
        return alphanumeric0()(src).and_then([](const auto &res) {
            if (res.value.size() < N)
                return not_parsed<decltype(res.value),
                                  typename decltype(res.rest)::value_type>(
                    "Do not enough symbols");

            return parsed(res.value, res.rest);
        });
    }};
}

/// @brief Take at least one alphanumeric symbol from source string
/// @return parser result
inline auto alphanumeric1() { return alphanumeric(at_least<1>()); }

}
