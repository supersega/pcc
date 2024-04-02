#pragma once

#include <pcc/core/common.hpp>
#include <variant>

namespace pcc {
template <typename S>
struct some {
    using value_t = S;

    explicit some(S s) : value(s) {}
    S value;
};

template <typename T>
struct is_some final : std::false_type {};

template <typename S>
struct is_some<some<S>> final : std::true_type {};

template <typename T>
inline constexpr bool is_some_v = is_some<T>::value;

struct none {};

template <typename U>
using enable_if_option_arg_t =
    typename std::enable_if_t<is_some_v<U> || std::is_same_v<U, none>>;

template <typename S>
struct option;

template <typename T>
struct is_option final : std::false_type {};

template <typename S>
struct is_option<option<S>> final : std::true_type {};

template <typename T>
inline constexpr bool is_option_v = is_option<T>::value;

template <typename T>
struct option final {
    using some_t = some<T>;
    using none_t = none;
    using some_value_t = typename some<T>::value_t;

    template <typename U, enable_if_option_arg_t<U>...>
    explicit constexpr option(U u) : opt(u) {}

    constexpr auto is_some() const -> bool {
        return std::holds_alternative<some_t>(opt);
    }

    constexpr auto value_unsafe() const -> T {
        return std::get<some_t>(opt).value;
    }

    template <typename F>
    auto map(F op) const {
        if constexpr (!std::is_invocable_v<F, typename some_t::value_t>)
            static_assert(detail::always_false_v<F>,
                          "Can not map with op, because it is not invocable "
                          "with success value");

        using op_res_t = decltype(op(std::declval<typename some_t::value_t>()));

        if constexpr (std::is_void_v<op_res_t>)
            static_assert(detail::always_false_v<F>,
                          "Operation can not return void");

        if constexpr (is_option_v<op_res_t>)
            static_assert(detail::always_false_v<F>,
                          "Operation for map should not return option<U>, use "
                          "and_then instead");

        using option_t = option<op_res_t>;

        if (is_some())
            return option_t(some(op(std::get<some_t>(opt).value)));

        return option_t(none_t{});
    }

    template <typename F>
    auto and_then(F op) const {
        if constexpr (!std::is_invocable_v<F, some_value_t>)
            static_assert(detail::always_false_v<F>,
                          "Can not map with op, because it is not invocable "
                          "with success value");

        using op_res_t = decltype(op(std::declval<some_value_t>()));

        if constexpr (!is_option_v<op_res_t>)
            static_assert(detail::always_false_v<F>,
                          "Operation should return result<U, E>");

        using option_t = option<typename op_res_t::some_value_t>;

        if (is_some())
            return op(std::get<some_t>(opt).value);

        return option_t(none_t{});
    }

    template <typename D>
    auto unwrap_or_else(D def) const {
        if (!is_some())
            return def();

        return value_unsafe();
    }

private:
    std::variant<some_t, none_t> opt;
};

template <typename U>
option(some<U>) -> option<U>;
} // namespace pcc
