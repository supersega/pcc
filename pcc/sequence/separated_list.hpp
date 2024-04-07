#pragma once

#include <pcc/core/common.hpp>
#include <pcc/parsers/parser.hpp>

#include <vector>

namespace pcc {

template <ParserFn ElementParser, ParserFn SeparatorParser>
auto separated_list(ElementParser ep, SeparatorParser sp) {
    return parser{[ep, sp](auto src) {
        using value_t = decltype(ep(src).value_unsafe().value);
        using char_t = typename decltype(src)::value_type;

        std::vector<value_t> vec;
        auto rest = detail::to_view_string(src);

        auto res = ep(rest);
        if (!res.is_success())
            return not_parsed<std::vector<value_t>, char_t>(
                "Failed to apply separated_list combinator");

        vec.push_back(res.value_unsafe().value);
        rest = res.value_unsafe().rest;

        while (true) {
            auto sep_res = sp(rest);
            if (!sep_res.is_success())
                break;

            auto ep_res = ep(sep_res.value_unsafe().rest);
            if (!ep_res.is_success())
                break;

            vec.push_back(ep_res.value_unsafe().value);
            rest = ep_res.value_unsafe().rest;
        }

        return parsed(std::move(vec), rest);
    }};
}
} // namespace pcc
