#pragma once

#include <pcc/parsers/parser.hpp>

namespace pcc {

/// @brief Parse with escape symbols
/// @param normal Normal parser
/// @param control Control symbol
/// @param escape Escape parser
template <ParserFn N, ParserFn Escape>
auto escaped(N &&normal, char control, Escape &&escape) {
    return parser{[normal = std::forward<N>(normal), control,
                   escape = std::forward<Escape>(escape)](const auto &src) {
        auto src_view = detail::to_view_string(src);
        using char_t = typename decltype(src_view)::value_type;

        auto local_view = src_view;
        while (!local_view.empty()) {
            auto res = normal(local_view);

            if (res.is_success()) {
                auto [value, rest] = res.value_unsafe();
                if (rest.empty())
                    return parsed(src_view, rest);
                if (rest.size() == local_view.size()) {
                    auto distance =
                        std::distance(src_view.begin(), rest.begin());
                    return parsed(src_view.substr(0, distance),
                                  src_view.substr(distance, src_view.size()));
                } else
                    local_view = rest;
            } else {
                if (local_view[0] == control) {
                    if (local_view.size() == 1)
                        return not_parsed<decltype(src_view), char_t>(
                            "End with control symbol");

                    auto aux = local_view.substr(1, local_view.size());
                    auto res_esc = escape(aux);
                    if (res_esc.is_success()) {
                        auto [value, rest] = res_esc.value_unsafe();
                        if (rest.empty())
                            return parsed(src_view, rest);
                        else
                            local_view = rest;
                    } else {
                        return not_parsed<decltype(src_view), char_t>(
                            "Not escapable symbol");
                    }
                } else {
                    auto distance =
                        std::distance(src_view.begin(), local_view.begin());
                    if (distance == 0) {
                        return not_parsed<decltype(src_view), char_t>(
                            "Non normal or control symbol");
                    }

                    return parsed(src_view.substr(0, distance),
                                  src_view.substr(distance, src_view.size()));
                }
            }
        }

        return parsed(src_view,
                      src_view.substr(src_view.size(), src_view.size()));
    }};
}
} // namespace pcc
