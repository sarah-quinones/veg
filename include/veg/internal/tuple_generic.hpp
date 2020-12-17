#ifndef VEG_TUPLE_GENERIC_HPP_DUSBI7AJS
#define VEG_TUPLE_GENERIC_HPP_DUSBI7AJS

#include "veg/internal/type_traits.hpp"
#include "veg/internal/tuple_spec.hpp"
#include "veg/internal/integer_seq.hpp"
#include "veg/internal/meta_int.hpp"

#include "veg/internal/std.hpp"

namespace veg {
namespace internal {
namespace tuple {
template <typename T, bool Default_Ctor /*true*/>
struct tuple_leaf_base {
  constexpr tuple_leaf_base() = default;
  constexpr explicit tuple_leaf_base(T&& arg) : elem(VEG_FWD(arg)) {}
  T elem = {};
};

template <typename T>
struct tuple_leaf_base<T, false> {
  constexpr explicit tuple_leaf_base(T&& arg) : elem(VEG_FWD(arg)) {}
  T elem;
};

template <typename T, usize I>
struct tuple_leaf
    : tuple_leaf_base<T, std::is_default_constructible<T>::value> {
  constexpr explicit tuple_leaf(T&& arg)
      : tuple_leaf_base<T, std::is_default_constructible<T>::value>{
            VEG_FWD(arg)} {}
};

template <meta::category_e C>
struct get_leaf_impl;

template <usize I, typename T>
auto get_leaf_type(tuple_leaf<T, I> const& arg) -> T;

template <>
struct get_leaf_impl<meta::category_e::mut_lval> {
  template <usize I, typename T>
  static constexpr auto apply(tuple_leaf<T, I>& arg) -> T& {
    return arg.elem;
  }
};
template <>
struct get_leaf_impl<meta::category_e::cst_lval> {
  template <usize I, typename T>
  static constexpr auto apply(tuple_leaf<T, I> const& arg) -> T const& {
    return arg.elem;
  }
};
template <>
struct get_leaf_impl<meta::category_e::mut_rval> {
  template <usize I, typename T>
  static constexpr auto apply(tuple_leaf<T, I>&& arg) -> T&& {
    return static_cast<tuple_leaf<T, I>&&>(arg).elem;
  }
};
template <>
struct get_leaf_impl<meta::category_e::cst_rval> {
  template <usize I, typename T>
  static constexpr auto apply(tuple_leaf<T, I> const& arg) -> T const&& {
    return static_cast<tuple_leaf<T, I> const&&>(arg).elem;
  }
};

template <typename ISeq, typename... Ts>
struct tuple_impl;

template <usize... Is, typename... Ts>
struct tuple_impl<meta::index_sequence<Is...>, Ts...> : tuple_leaf<Ts, Is>... {
  constexpr tuple_impl() = default;
  constexpr explicit tuple_impl(Ts&&... args)
      : tuple_leaf<Ts, Is>{static_cast<Ts&&>(args)}... {}
};

template <usize I, typename T, bool Use_Spec>
struct tuple_element {};

template <usize I, typename... Ts>
struct tuple_element<I, veg::tuple<Ts...>, false> {
  using type =
      decltype(tuple::get_leaf_type<I>(VEG_DECLVAL(veg::tuple<Ts...>&).m_impl));
};

template <usize I, typename... Ts>
struct tuple_element<I, veg::tuple<Ts...>, true> {
  using type = typename get_impl<I>::template type<Ts...>;
};
} // namespace tuple
} // namespace internal

template <typename... Ts>
struct tuple {
  constexpr tuple() = default;
  constexpr tuple /* NOLINT(hicpp-explicit-conversions) */ (
      Ts... args) noexcept(meta::all_of({std::is_move_constructible<Ts>::
                                             value...}))
      : m_impl(static_cast<Ts&&>(args)...) {}

  template <usize I, typename T>
  friend constexpr auto get(T&& tup) noexcept -> VEG_REQUIRES_RET2(
      (I < sizeof...(Ts) && meta::is_same_v<meta::remove_cvref_t<T>, tuple>),
      decltype(internal::tuple::get_leaf_impl<meta::value_category<
                   T&&>::value>::template apply<I>(VEG_FWD(tup).m_impl))) {
    return internal::tuple::get_leaf_impl<meta::value_category<T&&>::value>::
        template apply<I>(VEG_FWD(tup).m_impl);
  }

  template <i64 I>
  constexpr auto operator[](fix<I> /*arg*/) & noexcept -> VEG_REQUIRES_RET(
      (I < sizeof...(Ts)) && (I >= 0),
      decltype(internal::tuple::adl_get<I>(*this))) {
    return internal::tuple::adl_get<I>(*this);
  }
  template <i64 I>
  constexpr auto operator[](fix<I> /*arg*/) const& noexcept -> VEG_REQUIRES_RET(
      (I < sizeof...(Ts)) && (I >= 0),
      decltype(internal::tuple::adl_get<I>(*this))) {
    return internal::tuple::adl_get<I>(*this);
  }
  template <i64 I>
  constexpr auto operator[](fix<I> /*arg*/) && noexcept -> VEG_REQUIRES_RET(
      (I < sizeof...(Ts)) && (I >= 0),
      decltype(internal::tuple::adl_get<I>(static_cast<tuple&&>(*this)))) {
    return internal::tuple::adl_get<I>(static_cast<tuple&&>(*this));
  }
  template <i64 I>
  constexpr auto operator[](fix<I> /*arg*/) const&& noexcept
      -> VEG_REQUIRES_RET(
          (I < sizeof...(Ts)) && (I >= 0),
          decltype(
              internal::tuple::adl_get<I>(static_cast<tuple const&&>(*this)))) {
    return internal::tuple::adl_get<I>(static_cast<tuple const&&>(*this));
  }

private:
  template <usize, typename, bool>
  friend struct internal::tuple::tuple_element;

  internal::tuple::tuple_impl<meta::make_index_sequence<sizeof...(Ts)>, Ts...>
      m_impl;
};

VEG_CPP17(template <typename... Ts> tuple(Ts...) -> tuple<Ts...>;)

} // namespace veg

#endif /* end of include guard VEG_TUPLE_GENERIC_HPP_DUSBI7AJS */
