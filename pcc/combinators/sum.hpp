#pragma once

#include <pcc/parsers/parser.hpp>
#include <variant>

#include <pcc/core/common.hpp>
#include <pcc/core/parse_result.hpp>

namespace pcc {

namespace detail {

template <typename Variant, ParserFn Parser>
auto as_sum(Parser p) {
    return parser{[p](const auto &src) {
        auto res = p(src);
        if (res.is_success())
            return pcc::parsed(Variant{res.value_unsafe().value},
                               res.value_unsafe().rest);

        using char_t = typename decltype(res.value_unsafe().rest)::value_type;
        return pcc::not_parsed<Variant, char_t>("No suitable variant to parse");
    }};
}

template <typename Variant, ParserFn... Parsers>
auto sum(Parsers... ps) {
    return alternative(as_sum<Variant>(ps)...);
}

} // namespace detail

template <typename... Type, ParserFn... Parsers>
auto sum(Parsers... ps) {
    using sum_t = std::variant<Type...>;
    return detail::sum<sum_t>(ps...);
};
} // namespace pcc
