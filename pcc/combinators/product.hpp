#pragma once
#include <pcc/core/common.hpp>
#include <pcc/parsers/parser.hpp>

namespace pcc {
template <ParserFn Parser>
auto product(Parser p) {
    return parser{[p](const auto &src) {
        return p(src).and_then(
            [](auto v) { return parsed(std::tuple(v.value), v.rest); });
    }};
};

template <ParserFn Parser1, ParserFn Parser2>
auto product(Parser1 p1, Parser2 p2) {
    return parser{[p1, p2](const auto &src) {
        return product(p1)(src).and_then([p2](auto v) {
            return product(p2)(v.rest).and_then([v](auto u) {
                return parsed(std::tuple_cat(v.value, u.value), u.rest);
            });
        });
    }};
};

template <ParserFn Parser1, ParserFn Parser2, ParserFn... Parsers>
auto product(Parser1 p1, Parser2 p2, Parsers... rest) {
    return product(product(p1, p2), rest...);
};

namespace detail {
template <typename Target, typename Tuple, size_t... Is>
constexpr auto into_struct_impl(Tuple t, std::index_sequence<Is...>) {
    return Target{std::get<Is>(t)...};
}

template <typename Target, typename Tuple>
auto into_struct(Tuple t) {
    constexpr auto size =
        std::tuple_size<std::remove_reference_t<Tuple>>::value;
    return into_struct_impl<Target>(t, std::make_index_sequence<size>{});
}
} // namespace detail

template <typename Builder, ParserFn... Parsers>
auto product(Builder fn, Parsers... rest) {
    return parser{[fn, rest...](auto src) {
        return product(rest...)(src).and_then(
            [fn](auto t) { return parsed(std::apply(fn, t.value), t.rest); });
    }};
};

template <typename Product, ParserFn... Parsers>
auto product(Parsers... rest) {
    return product([](auto... args) { return Product{args...}; }, rest...);
};
} // namespace pcc
