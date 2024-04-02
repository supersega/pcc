#pragma once

#include <vector>

#include <pcc/core/common.hpp>

namespace pcc {

template <ParserFn Parser>
auto many0(Parser p) {
    return parser{[p](const auto &src) {
        using value_t = decltype(p(src).value_unsafe().value);

        std::vector<value_t> vec;
        auto rest = detail::to_view_string(src);

        while (true) {
            auto res = p(rest);
            if (!res.is_success())
                break;

            vec.push_back(res.value_unsafe().value);
            rest = res.value_unsafe().rest;
        }

        return parsed(std::move(vec), rest);
    }};
}

template <ParserFn Parser, typename std::size_t N>
auto many(detail::at_least<N>, Parser p) {
    return parser{[p](const auto &src) {
        return many0(p)(src).and_then([](const auto &parse_result) {
            using char_t = typename decltype(parse_result.rest)::value_type;
            using value_t = decltype(parse_result.value);

            if (parse_result.value.size() < N)
                return not_parsed<value_t, char_t>(
                    "Failed to apply many combinator");

            return parsed(std::move(parse_result.value), parse_result.rest);
        });
    }};
}
} // namespace pcc
