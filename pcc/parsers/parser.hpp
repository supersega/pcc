#pragma once

#include <pcc/core/parse_result.hpp>
#include <type_traits>

#include <pcc/core/common.hpp>

namespace pcc {
template <typename P>
struct parser : P {
    template <typename T>
    parser(T &&t) : P(std::forward<T>(t)) {}

    using P::operator();
};

template <typename T>
parser(T) -> parser<std::decay_t<T>>;

template <typename T>
struct is_parser final : std::false_type {};

template <typename P>
struct is_parser<parser<P>> final : std::true_type {};

template <typename T>
inline constexpr bool is_parser_v = is_parser<T>::value;

template <class T>
concept ParserFn = is_parser_v<T>;

// template<typename Pl, typename Pr, std::enable_if_t<is_parser_v<Pl> &&
// is_parser_v<Pr>>...>
template <ParserFn Pl, ParserFn Pr>
inline auto operator<<=(const Pl &l, const Pr &r) {
    return parser{[l, r](const auto &src) {
        return l(src).and_then([r](auto v) { return r(v.rest); });
    }};
}

template <ParserFn Pl, ParserFn Pr>
inline auto operator>>=(const Pl &l, const Pr &r) {
    return parser{[l, r](const auto &src) {
        return l(src).and_then([r](const auto &lv) {
            return r(lv.rest).map([res = lv.value](const auto &rv) {
                return detail::parsed(res, rv.rest);
            });
        });
    }};
}
} // namespace pcc
