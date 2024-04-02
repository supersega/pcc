#pragma once

#include <string_view>

#include <pcc/core/result.hpp>

namespace pcc {
namespace detail {

/// @brief Parsing result with value and rest of source string
template <typename S, typename Ch>
struct parsed final {
    using value_t = S;
    explicit constexpr parsed(S s, std::basic_string_view<Ch> r)
        : value(s), rest(r) {}

    S value;                         ///< result
    std::basic_string_view<Ch> rest; ///< source string to parse
};
} // namespace detail

/// @brief Parsed result type either with value and rest of source string or error message
template <typename S, typename Ch>
using parse_result = result<detail::parsed<S, Ch>, std::string_view>;

/// @brief Create parsed result from value and rest of source string
template <typename S, typename Ch>
auto parsed(S s, std::basic_string_view<Ch> r) {
    return parse_result<S, Ch>(success(detail::parsed(s, r)));
}

/// @brief Create not parsed result with error message
template <typename S, typename Ch>
auto not_parsed(std::string_view msg) {
    return parse_result<S, Ch>(error(std::string_view(msg)));
}
} // namespace pcc
