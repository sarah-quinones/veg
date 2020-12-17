#ifndef VEG_TUPLE_HPP_B8PHUNWES
#define VEG_TUPLE_HPP_B8PHUNWES

#include "veg/internal/tuple_generic.hpp"

namespace veg {
namespace internal {
namespace get {

template <typename T>
void get() = delete;

template <typename T, size_t I, typename Enable = void>
struct has_member_get : std::false_type {};

template <typename T, size_t I>
struct has_member_get<
    T,
    I,
    meta::void_t<decltype(VEG_DECLVAL(T).template get<I>())>> : std::true_type {
};

template <typename T, size_t I, typename Enable = void>
struct has_adl_get : std::false_type {};

template <typename T, size_t I>
struct has_adl_get<T, I, meta::void_t<decltype(get<I>(VEG_DECLVAL(T)))>>
    : std::true_type {};

template <
    typename T,
    size_t I,
    i64 which = meta::first_true< //
        has_member_get<T, I>,     //
        has_adl_get<T, I>         //
        >::value                  //
    >
struct has_get;

template <typename T, size_t I>
struct has_get<T, I, meta::not_found> : std::false_type {
  static void apply(T&& /*arg*/) {}
};
template <typename T, size_t I>
struct has_get<T, I, 0> : std::true_type {
  constexpr static auto apply(T&& arg)
      -> decltype(VEG_FWD(arg).template get<I>()) {
    return VEG_FWD(arg).template get<I>();
  }
};
template <typename T, size_t I>
struct has_get<T, I, 1> : std::true_type {
  constexpr static auto apply(T&& arg) -> decltype(get<I>(VEG_FWD(arg))) {
    return get<I>(VEG_FWD(arg));
  }
};

} // namespace get
} // namespace internal

namespace fn {
template <i64 I>
struct get_fn {
  template <typename T>
  constexpr auto operator()(T&& arg) const noexcept -> VEG_REQUIRES_RET(
      (internal::get::has_get<T, static_cast<size_t>(I)>::value),
      decltype(internal::get::has_get<T, static_cast<size_t>(I)>::apply(
          VEG_FWD(arg)))) {
    return internal::get::has_get<T, static_cast<size_t>(I)>::apply(
        VEG_FWD(arg));
  }
};
} // namespace fn
namespace { /* NOLINT */
template <i64 I>
constexpr auto const& get /* NOLINT */ =
    ::veg::meta::internal::static_const<fn::get_fn<I>>::value;
}

namespace make {
namespace fn {
struct tuple_fn {
  template <typename... Ts>
  constexpr auto operator()(Ts&&... args) const noexcept -> VEG_REQUIRES_RET(
      (VEG_ALL_OF(
          std::is_constructible<meta::remove_cvref_t<Ts>, Ts&&>::value)),
      tuple<meta::remove_cvref_t<Ts>...>) {
    return {VEG_FWD(args)...};
  }
};
} // namespace fn
VEG_ODR_VAR(tuple, fn::tuple_fn);

} // namespace make
} // namespace veg

namespace std {
template <typename... Ts>
struct tuple_size<veg::tuple<Ts...>>
    : integral_constant<veg::meta::size_t, sizeof...(Ts)> {};

template <veg::meta::size_t I, typename... Ts>
struct tuple_element<I, veg::tuple<Ts...>>
    : veg::internal::tuple::tuple_element<
          I,
          veg::tuple<Ts...>,
          (sizeof...(Ts) < VEG_TUPLE_SPECIALIZATION_COUNT)> {};
} // namespace std

#endif /* end of include guard VEG_TUPLE_HPP_B8PHUNWES */
