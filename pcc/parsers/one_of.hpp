#pragma once

#include <pcc/parsers/parser.hpp>

namespace pcc {

/// @brief Parser that tries to parse one of the characters in the input
/// @param input The characters to try to parse
/// @return A parser that tries to parse one of the characters in the input
template <typename Input>
auto one_of(Input &&input) {
    return parser{[input = std::forward<Input>(input)](const auto &src) {
        auto src_view = detail::to_view_string(src);
        auto input_view = detail::to_view_string(input);
        using char_t = typename decltype(src_view)::value_type;

        auto it =
            std::find(input_view.begin(), input_view.end(), src_view.front());
        if (it != input_view.end()) {
            return parsed(src_view.front(), src_view.substr(1));
        }

        return not_parsed<char_t, char_t>("A character is not in the input");
    }};
}

} // namespace pcc
