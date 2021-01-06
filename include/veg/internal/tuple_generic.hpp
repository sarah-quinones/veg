#ifndef VEG_TUPLE_GENERIC_HPP_DUSBI7AJS
#define VEG_TUPLE_GENERIC_HPP_DUSBI7AJS

#include "veg/internal/type_traits.hpp"
#include "veg/internal/integer_seq.hpp"
#include "veg/internal/integer_seq.hpp"
#include "veg/internal/meta_int.hpp"
#include "veg/internal/storage.hpp"
#include <utility> // std::tuple_{size,element}

namespace veg {

template <typename... Ts>
struct tuple;

namespace internal {
namespace tuple {

template <typename T>
auto get() = delete;
template <usize I, typename T>
auto adl_get(T&& arg) noexcept(noexcept(get<I>(VEG_FWD(arg))))
    -> decltype(auto) {
  return get<I>(VEG_FWD(arg));
}

template <usize I, typename T>
struct tuple_leaf : storage::storage<T> {
  using storage::storage<T>::storage;
};

template <typename ISeq, typename... Ts>
struct tuple_impl;

template <usize... Is, typename... Ts>
struct tuple_impl<meta::index_sequence<Is...>, Ts...> : tuple_leaf<Is, Ts>... {
  constexpr tuple_impl() = default;
  constexpr explicit tuple_impl(meta::remove_cv_t<Ts>&&... args)
      : tuple_leaf<Is, Ts>{VEG_FWD(args)}... {}
};

template <usize I>
struct pack_ith_elem {
  template <typename... Ts>
  using type = decltype(
      storage::get_inner<meta::category_e::own>::
          template with_idx<usize, tuple_leaf>::template get_type<
              I>(VEG_DECLVAL(
              tuple_impl<meta::make_index_sequence<sizeof...(Ts)>, Ts...>)));
};

template <typename T>
VEG_TRAIT_VAR is_tuple = false;
template <typename... Ts>
VEG_TRAIT_VAR is_tuple<veg::tuple<Ts...>> = true;

template <typename T>
auto get_inner(T&& tup) noexcept -> auto&& {
  return VEG_FWD(tup).m_impl;
}

template <usize I, typename... Ts>
constexpr auto ith_impl(veg::tuple<Ts...>& tup) noexcept -> auto& {

  return internal::storage::get_inner<meta::category_e::ref_mut>::
      template with_idx<usize, internal::tuple::tuple_leaf>::template impl<I>(
          tuple::get_inner(tup));
}

template <typename... Ts, typename... Us, usize... Is>
constexpr void swap_impl(
    veg::tuple<Ts...>& ts,
    veg::tuple<Us...>& us,
    meta::index_sequence<Is...> /*seq*/) {

  int dummy[] = {(tuple::ith_impl<Is>(ts).swap(tuple::ith_impl<Is>(us)), 0)...};
  (void)dummy;
}

namespace adl {

template <typename...>
struct tuple_base {};

VEG_TEMPLATE(
    (usize I, typename T),
    requires(
        is_tuple<meta::remove_cvref_t<T>> &&
        (I < std::tuple_size<meta::remove_cvref_t<T>>::value)),
    constexpr auto get,
    (tup, T&&))
noexcept(meta::lvalue_reference<T>) -> decltype(auto) {
  return internal::storage::get_inner<meta::value_category<T>::value>::
      template with_idx<usize, internal::tuple::tuple_leaf>::template apply<I>(
          tuple::get_inner(VEG_FWD(tup)));
}

VEG_TEMPLATE(
    (typename... Ts, typename... Us),
    requires //
    VEG_ALL_OF(
        ((meta::reference<Ts> //
          && VEG_SAME_AS(Ts, Us)) ||
         meta::is_swappable<Ts&, Us&>::value)),
    constexpr void swap,
    (ts, veg::tuple<Ts...>&),
    (us, veg::tuple<Us...>&))
noexcept(VEG_ALL_OF(meta::is_nothrow_swappable<
                    internal::storage::storage<Ts>&,
                    internal::storage::storage<Us>&>::value)) {
  return tuple::swap_impl(ts, us, meta::make_index_sequence<sizeof...(Ts)>{});
}

} // namespace adl

} // namespace tuple
} // namespace internal

template <typename... Ts>
struct tuple : internal::tuple::adl::tuple_base<Ts...> {
  constexpr tuple() = default;
  constexpr tuple /* NOLINT(hicpp-explicit-conversions) */
      (meta::remove_cv_t<Ts>... args) noexcept
      : m_impl(VEG_FWD(args)...) {}

  VEG_CVREF_DUPLICATE(
      VEG_TEMPLATE(
          (i64 I),
          requires(I < sizeof...(Ts) && (I >= 0)),
          constexpr auto
          operator[],
          (/*arg*/, fix<I>)),
      internal::tuple::adl_get<I>,
      ());

  VEG_CVREF_DUPLICATE(
      constexpr auto as_ref(),
      tuple::as_ref_impl,
      (, meta::make_index_sequence<sizeof...(Ts)>{}));

private:
  internal::tuple::tuple_impl<meta::make_index_sequence<sizeof...(Ts)>, Ts...>
      m_impl;

  template <usize I, typename Self>
  static constexpr auto ith_ref(Self&& tup) noexcept -> auto&& {

    return internal::storage::get_inner<meta::value_category<Self&&>::value>::
        template with_idx<usize, internal::tuple::tuple_leaf>::template apply<
            I>(VEG_FWD(tup).m_impl);
  }

  template <typename Self, usize... Is>
  static constexpr auto
  as_ref_impl(Self&& self, meta::index_sequence<Is...> /*seq*/) noexcept {
    return tuple<decltype(tuple::ith_ref<Is>(VEG_FWD(self)))...>{
        tuple::ith_ref<Is>(VEG_FWD(self))...};
  }

  template <typename T>
  friend auto internal::tuple::get_inner(T&& tup) noexcept -> auto&&;
};

VEG_CPP17(template <typename... Ts> tuple(Ts...) -> tuple<Ts...>;)

} // namespace veg

#endif /* end of include guard VEG_TUPLE_GENERIC_HPP_DUSBI7AJS */
