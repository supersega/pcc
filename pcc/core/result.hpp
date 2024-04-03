#pragma once

#include <type_traits>
#include <variant>

#include <pcc/core/option.hpp>

namespace pcc {
namespace detail {
template <typename S>
struct success final {
    using value_t = S;

    explicit success(S v) : value(std::move(v)) {}

    template <typename U, std::enable_if_t<!std::is_same_v<U, S> &&
                                           std::is_convertible_v<U, S>>...>
    explicit success(const U &u) : value(u) {}

    value_t value;
};

template <typename E>
struct error final {
    using value_t = E;

    explicit error(E v) : value(std::move(v)) {}

    template <typename U, std::enable_if_t<!std::is_same_v<U, E> &&
                                           std::is_convertible_v<U, E>>...>
    explicit error(const U &u) : value(u) {}

    value_t value;
};
} // namespace detail

template <typename S>
constexpr auto success(S value) -> detail::success<S> {
    return detail::success<S>(std::move(value));
}

template <typename E>
constexpr auto error(E value) -> detail::error<E> {
    return detail::error<E>(std::move(value));
}

template <typename S, typename E>
struct result;

template <typename T>
struct is_result final : std::false_type {};

template <typename S, typename E>
struct is_result<result<S, E>> final : std::true_type {};

template <typename T>
inline constexpr bool is_result_v = is_result<T>::value;

template <typename S, typename E>
struct result final {
    using success_t = detail::success<S>;
    using error_t = detail::error<E>;
    using success_value_t = typename success_t::value_t;
    using error_value_t = typename error_t::value_t;

    template <typename U, std::enable_if_t<std::is_same_v<U, success_t> ||
                                           std::is_same_v<U, error_t>>...>
    explicit constexpr result(U u) : either(u) {}

    auto as_option() const -> option<S> {
        if (is_success())
            return option<S>{some(std::get<success_t>(either).value)};

        return option<S>{none{}};
    }

    constexpr auto is_success() const -> bool {
        return std::holds_alternative<success_t>(either);
    }

    constexpr auto value_unsafe() const -> S {
        return std::get<success_t>(either).value;
    }

    constexpr auto error_unsafe() const -> E {
        return std::get<error_t>(either).value;
    }

    template <typename F>
    auto map(F op) const {
        if constexpr (!std::is_invocable_v<F, typename success_t::value_t>)
            static_assert(detail::always_false_v<F>,
                          "Can not map with op, because it is not invocable "
                          "with success value");

        using op_res_t =
            decltype(op(std::declval<typename success_t::value_t>()));

        if constexpr (std::is_void_v<op_res_t>)
            static_assert(detail::always_false_v<F>,
                          "Operation can not return void");

        if constexpr (is_result_v<op_res_t>)
            static_assert(detail::always_false_v<F>,
                          "Operation for map should not return result<U, E>, "
                          "use and_then instead");

        using result_t = result<op_res_t, error_value_t>;

        if (is_success())
            return result_t(success(op(std::get<success_t>(either).value)));

        return result_t(error(std::get<error_t>(either).value));
    }

    template <typename F>
    auto map_error(F op) const {
        if constexpr (!std::is_invocable_v<F, typename error_t::value_t>)
            static_assert(detail::always_false_v<F>,
                          "Can not map with op, because it is not invocable "
                          "with error value");

        using op_res_t =
            decltype(op(std::declval<typename error_t::value_t>()));

        if constexpr (std::is_void_v<op_res_t>)
            static_assert(detail::always_false_v<F>,
                          "Operation can not return void");

        using result_t = result<success_value_t, op_res_t>;

        if (is_success())
            return result_t(success(std::get<success_t>(either).value));

        return result_t(error(op(std::get<error_t>(either).value)));
    }

    template <typename F>
    auto and_then(F op) const {
        if constexpr (!std::is_invocable_v<F, typename success_t::value_t>)
            static_assert(detail::always_false_v<F>,
                          "Can not map with op, because it is not invocable "
                          "with success value");

        using op_res_t =
            decltype(op(std::declval<typename success_t::value_t>()));

        if constexpr (!is_result_v<op_res_t>)
            static_assert(detail::always_false_v<F>,
                          "Operation should return result<U, E>");

        if constexpr (!std::is_same_v<typename op_res_t::error_t, error_t>)
            static_assert(detail::always_false_v<F>,
                          "Error type should be same");

        using result_t =
            result<typename op_res_t::success_value_t, error_value_t>;

        if (is_success())
            return op(std::get<success_t>(either).value);

        return result_t(error(std::get<error_t>(either).value));
    }

    template <typename D, typename F>
    auto map_or_else(D def, F op) const {
        if (!is_success())
            return def();

        return map(op).value_unsafe();
    }

private:
    std::variant<detail::success<S>, detail::error<E>> either;
};
} // namespace pcc
