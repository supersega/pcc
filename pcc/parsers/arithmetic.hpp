#pragma once

#include <pcc/parsers/parser.hpp>
#include <charconv>
#include <locale>

namespace pcc {
namespace detail {
template <typename T, std::enable_if_t<std::is_integral_v<T>>...>
auto from_chars(const char *begin, const char *end, T &value, int base = 10) {
    return std::from_chars(begin, end, value, base);
}

template <typename T>
T strtofp(const char *begin, char **_end, const T &);

template <>
inline double strtofp<double>(const char *begin, char **end, const double &) {
    return std::strtod(begin, end);
}

template <>
inline float strtofp<float>(const char *begin, char **end, const float &) {
    return std::strtof(begin, end);
}

template <typename T, std::enable_if_t<std::is_floating_point_v<T>>...>
auto from_chars(const char *begin, const char *_end,
                T &value) -> std::from_chars_result {
    char *end;
    value = strtofp(begin, &end, T{});
    auto err = begin != end ? std::errc{} : std::errc::invalid_argument;
    // underflow
    return std::from_chars_result{end, err};
}
} // namespace detail

template <typename T, std::enable_if_t<std::is_arithmetic_v<T>>...>
auto arithmetic() {
    return parser{[](const auto &src) {
        auto src_view = detail::to_view_string(src);
        using char_t = typename decltype(src_view)::value_type;

        if (src_view.empty())
            return not_parsed<T, char_t>(
                "Can not parse arithmetic value form empty string");

        if (auto loc = std::locale(); std::isspace(src_view[0], loc))
            return not_parsed<T, char_t>(
                "Source string should not start from space");

        T result;
        auto [ptr, e] = detail::from_chars(
            src_view.data(), src_view.data() + src_view.size(), result);

        if (e == std::errc{})
            return parsed(result, std::string_view(ptr));

        return not_parsed<T, char_t>("Can not parse double value");
    }};
}

template <typename T, std::enable_if_t<std::is_integral_v<T>>...>
auto integral(int base = 10) {
    return parser{[base](const auto &src) {
        auto src_view = detail::to_view_string(src);
        using char_t = typename decltype(src_view)::value_type;

        if (src_view.empty())
            return not_parsed<T, char_t>(
                "Can not parse integral value form empty string");

        if (auto loc = std::locale(); std::isspace(src_view[0], loc))
            return not_parsed<T, char_t>(
                "Source string should not start from space");

        T result;
        auto [ptr, e] = detail::from_chars(
            src_view.data(), src_view.data() + src_view.size(), result, base);

        if (e == std::errc{})
            return parsed(result, std::string_view(ptr));

        return not_parsed<T, char_t>("Can not parse integral value");
    }};
}
} // namespace pcc
