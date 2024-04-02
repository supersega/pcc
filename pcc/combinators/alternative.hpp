#pragma once
#include <pcc/parsers/parser.hpp>
#include <pcc/core/common.hpp>

namespace pcc {
template <ParserFn Parser1, ParserFn Parser2>
auto alternative(Parser1 p1, Parser2 p2) {
    return parser{[p1, p2](const auto &src) {
        auto res = p1(src);
        if (res.is_success())
            return res;

        return p2(src);
    }};
};

template <ParserFn Parser1, ParserFn Parser2, ParserFn... Parsers>
auto alternative(Parser1 p1, Parser2 p2, Parsers... rest) {
    return alternative(alternative(p1, p2), rest...);
};
} // namespace pcc
