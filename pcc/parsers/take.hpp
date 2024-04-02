#pragma once

#include <algorithm>
#include <pcc/parsers/parser.hpp>

#include <pcc/core/common.hpp>

namespace pcc {

/// @brief Take symbols from source string while predicate matches
/// @param pred Predicate to match
/// @return parser result
template <typename Pred>
auto take_while(Pred &&pred) {
    return parser{[pred = std::forward<Pred>(pred)](const auto &src) {
        auto src_view = detail::to_view_string(src);
        using char_t = typename decltype(src_view)::value_type;

        if (src_view.empty())
            return not_parsed<decltype(src_view), char_t>(
                "Can not take from empty string");

        auto first = src_view.begin();
        auto last = src_view.end();

        auto found = std::find_if_not(src_view.begin(), src_view.end(), pred);

        auto pos = std::distance(first, found);
        auto len = std::distance(first, last);
        return parsed(src_view.substr(0, pos), src_view.substr(pos, len));
    }};
}

/// @brief Take exactly N symbols from source string
/// @param exactly<N> where N is number of symbols to take
/// @return parser result
template <std::size_t N>
auto take(detail::exactly<N>) {
    return parser{[](const auto &src) {
        auto src_view = detail::to_view_string(src);
        using char_t = typename decltype(src_view)::value_type;

        if (src_view.empty())
            return not_parsed<decltype(src_view), char_t>(
                "Can not take from empty string");

        const auto len = src_view.size();
        if (len < N)
            return not_parsed<decltype(src_view), char_t>(
                "Can not take from string which is less N");

        return parsed(src_view.substr(0, N), src_view.substr(N, len));
    }};
}
} // namespace pcc
