#pragma once
#include <pcc/core/common.hpp>
#include <pcc/parsers/parser.hpp>

namespace pcc {
template <ParserFn Parser>
auto optional(Parser p) {
    return parser{[p](const auto &src) {
        auto res = p(src);
        return parsed(
            res.as_option().map([](auto pr) { return pr.value; }),
            res.map_or_else([&src] { return detail::to_view_string(src); },
                            [](auto pr) { return pr.rest; }));
    }};
}
} // namespace pcc
