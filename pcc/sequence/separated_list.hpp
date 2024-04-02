#pragma once

#include <pcc/core/common.hpp>

#include <vector>

namespace pcc {

template <typename Sep, ParserFn Parser>
auto separated_list(Sep sep, Parser p) {
    return parser{
        [p, sep](const auto &src) { return many0(p <<= symbol(sep))(src); }};
}
} // namespace pcc
