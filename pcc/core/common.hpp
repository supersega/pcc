#pragma once

#include <string_view>
#include <type_traits>

namespace pcc {
namespace detail {
struct always_false {
    static constexpr bool value = false;
};

template <typename...>
constexpr inline auto always_false_v = always_false::value;

template <bool... Values>
struct all_of_impl : std::true_type {};

template <bool... Values>
struct all_of_impl<true, Values...> : all_of_impl<Values...> {};

template <bool... Values>
struct all_of_impl<false, Values...> : std::false_type {};

template <template <class> class T, class... U>
using all_of = all_of_impl<T<U>::value...>;

template <template <class> class T, class... U>
inline constexpr auto all_of_v = all_of<T, U...>::value;

struct mystique {
    template <typename T>
    operator T() const;
};

template <typename T>
auto to_view_string(T src) {
    return std::basic_string_view(src);
}

template <typename Ch>
auto to_view_string(const std::basic_string<Ch> &src) {
    return std::basic_string_view<Ch>(src);
}

template <typename std::size_t N>
struct at_least {
    static constexpr auto size = N;
};

template <std::size_t N>
struct exactly final {
    static_assert(N > 0, "Please provide number > 0");
    static constexpr auto size = N;
};
} // namespace detail

template <typename std::size_t N>
constexpr auto at_least() -> detail::at_least<N> {
    return {};
}

template <std::size_t N>
constexpr auto exactly() -> detail::exactly<N> {
    return {};
}

} // namespace pcc
