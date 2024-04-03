#pragma once

#include <pcc/parsers/parser.hpp>

namespace pcc {

/// @brief Map the result of a parser using a function
/// @param fn The function to apply to the result of the parsers
/// @param p The parser to apply
/// @return A new parser that applies the function to the result of the parser
template <typename Map, ParserFn Parser>
auto map(Map &&fn, Parser &&p) {
    return parser{[fn = std::forward<Map>(fn),
                   p = std::forward<Parser>(p)](const auto &src) mutable {
        return p(src).map([fn = std::forward<Map>(fn)](const auto &v) {
            return pcc::detail::parsed{fn(v.value), v.rest};
        });
    }};
};

} // namespace pcc
