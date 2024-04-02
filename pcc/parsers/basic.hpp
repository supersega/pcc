#pragma once

#include <pcc/parsers/parser.hpp>
#include <algorithm>
#include <cctype>
#include <string_view>

#include <pcc/core/common.hpp>

namespace pcc {
template <typename Ch>
auto symbol(Ch ch) {
    return parser{[ch](const auto &src) {
        auto src_view = detail::to_view_string(src);
        static_assert(
            std::is_same_v<Ch, typename decltype(src_view)::value_type>,
            "Character type should be same as char for source string");

        if (src_view.empty())
            return not_parsed<Ch, Ch>("Can not parse value");

        if (src_view[0] == ch)
            return parsed(ch, src_view.substr(1, src_view.size()));

        return not_parsed<Ch, Ch>("Can not match symbol");
    }};
}

template <typename Tag>
constexpr auto tag(Tag tg) {
    return parser{[tg](const auto &src) constexpr {
        auto t = detail::to_view_string(tg);
        using char_t = typename decltype(t)::value_type;

        if (t.empty())
            return not_parsed<decltype(t), char_t>(
                "Tag can not be empty string");

        std::basic_string_view<char_t> src_view = src;
        if (src_view.find(t) == 0)
            return parsed(t, src_view.substr(t.size(), src_view.size()));

        return not_parsed<decltype(t), char_t>("Tag not found");
    }};
}

template <typename Ch>
constexpr auto tag(const std::basic_string<Ch> &t) {
    return tag(std::basic_string_view<Ch>(t));
}
} // namespace pcc
