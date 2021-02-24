#ifndef __VEG_TUPLE_GENERIC_HPP_DUSBI7AJS
#define __VEG_TUPLE_GENERIC_HPP_DUSBI7AJS

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
struct hidden_tag {};

template <typename T>
void get() = delete;
template <usize I, typename T>
HEDLEY_ALWAYS_INLINE auto adl_get(T&& arg)
    __VEG_DEDUCE_RET(get<I>(VEG_FWD(arg)));

template <usize I, typename T>
struct tuple_leaf : storage::storage<T> {
  using storage::storage<T>::storage;
};

template <typename ISeq, typename... Ts>
struct tuple_impl;

template <usize... Is, typename... Ts>
struct tuple_impl<meta::index_sequence<Is...>, Ts...> : tuple_leaf<Is, Ts>... {
  constexpr tuple_impl() = default;
  HEDLEY_ALWAYS_INLINE constexpr explicit tuple_impl(Ts&&... args) noexcept(
      meta::all_of({meta::nothrow_constructible<Ts, Ts&&>::value...}))
      : tuple_leaf<Is, Ts>{VEG_FWD(args)}... {}

  template <typename... Fn>
  HEDLEY_ALWAYS_INLINE constexpr explicit tuple_impl(
      hidden_tag /*unused*/,
      Fn&&... fns) noexcept(meta::all_of({meta::nothrow_invocable<Fn&&>::
                                              value...}))
      : tuple_leaf<Is, Ts>{0, VEG_FWD(fns)}... {}
};

template <usize I>
struct pack_ith_elem {
  template <typename... Ts>
  using type = decltype(
      storage::get_inner<meta::category_e::own>::
          template with_idx<usize, tuple_leaf>::template get_type<
              I>(__VEG_DECLVAL(
              tuple_impl<meta::make_index_sequence<sizeof...(Ts)>, Ts...>)));
};

template <typename T>
struct is_tuple : std::false_type {};
template <typename... Ts>
struct is_tuple<veg::tuple<Ts...>> : std::true_type {};

template <typename T>
HEDLEY_ALWAYS_INLINE auto get_inner(T&& tup) noexcept
    -> decltype((VEG_FWD(tup).m_impl))&& {
  return VEG_FWD(tup).m_impl;
}

template <usize I, typename... Ts>
HEDLEY_ALWAYS_INLINE constexpr auto
ith_impl(veg::tuple<Ts...>& tup) noexcept -> decltype(
    internal::storage::get_inner<meta::category_e::ref_mut>::template with_idx<
        usize,
        internal::tuple::tuple_leaf>::template impl<I>(tuple::get_inner(tup))) {

  return internal::storage::get_inner<meta::category_e::ref_mut>::
      template with_idx<usize, internal::tuple::tuple_leaf>::template impl<I>(
          tuple::get_inner(tup));
}

template <typename... Ts, typename... Us, usize... Is>
__VEG_CPP14(constexpr)
void swap_impl(
    veg::tuple<Ts...>& ts,
    veg::tuple<Us...>& us,
    meta::index_sequence<
        Is...> /*seq*/) noexcept(noexcept(meta::internal::int_arr{
    (tuple::ith_impl<Is>(ts).swap(tuple::ith_impl<Is>(us)), 0)...})) {
  (void)meta::internal::int_arr{
      (tuple::ith_impl<Is>(ts).swap(tuple::ith_impl<Is>(us)), 0)...};
}

namespace adl {

template <typename...>
struct tuple_base {};

VEG_TEMPLATE(
    (usize I, typename T),
    requires(
        is_tuple<meta::remove_cvref_t<T>>::value &&
        (I < std::tuple_size<meta::remove_cvref_t<T>>::value)),
    HEDLEY_ALWAYS_INLINE constexpr auto get,
    (tup, T&&))
__VEG_DEDUCE_RET(
    internal::storage::get_inner<meta::value_category<T>::value>::
        template with_idx<usize, internal::tuple::tuple_leaf>::template apply<
            I>(tuple::get_inner(VEG_FWD(tup))));

VEG_TEMPLATE(
    (typename... Ts, typename... Us),
    requires //
    __VEG_ALL_OF(
        ((meta::reference<Ts>::value //
          && __VEG_SAME_AS(Ts, Us)) ||
         meta::swappable<Ts&, Us&>::value)),
    HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) void swap,
    (ts, veg::tuple<Ts...>&),
    (us, veg::tuple<Us...>&))
noexcept(meta::all_of({(meta::nothrow_swappable<
                        internal::storage::storage<Ts>&,
                        internal::storage::storage<Us>&>::value)...})) {
  return tuple::swap_impl(ts, us, meta::make_index_sequence<sizeof...(Ts)>{});
}

} // namespace adl

} // namespace tuple
} // namespace internal

template <typename... Ts>
struct tuple : veg::internal::tuple::adl::tuple_base<Ts...> {
  constexpr tuple() = default;
  HEDLEY_ALWAYS_INLINE constexpr tuple /* NOLINT(hicpp-explicit-conversions) */
      (Ts... args) noexcept(
          meta::all_of({meta::nothrow_move_constructible<Ts>::value...}))
      : m_impl(VEG_FWD(args)...) {}

  VEG_TEMPLATE(
      (typename... Fns),
      requires __VEG_ALL_OF((
          meta::invocable<Fns&&>::value && //
          __VEG_SAME_AS(Ts, (meta::detected_t<meta::invoke_result_t, Fns&&>)))),
      HEDLEY_ALWAYS_INLINE constexpr tuple // NOLINT(hicpp-explicit-conversions)
      ,
      (/*tag*/, inplace_t),
      (... fns, Fns&&))
  noexcept
      : m_impl(internal::tuple::hidden_tag{}, VEG_FWD(fns)...) {}

  __VEG_CVREF_DUPLICATE(
      VEG_TEMPLATE(
          (i64 I),
          requires(I < sizeof...(Ts) && (I >= 0)),
          HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto
          operator[],
          (/*arg*/, fix<I>)),
      internal::tuple::adl_get<I>,
      ());

  __VEG_CVREF_DUPLICATE(
      HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto as_ref(),
      tuple::as_ref_impl,
      (, meta::make_index_sequence<sizeof...(Ts)>{}));

private:
  internal::tuple::tuple_impl<meta::make_index_sequence<sizeof...(Ts)>, Ts...>
      m_impl;

  template <usize I, typename Self>
  HEDLEY_ALWAYS_INLINE static constexpr auto
  ith_ref(Self&& tup) noexcept -> decltype(
      internal::storage::get_inner<meta::value_category<Self&&>::value>::
          template with_idx<usize, internal::tuple::tuple_leaf>::template apply<
              I>(VEG_FWD(tup).m_impl))&& {

    return internal::storage::get_inner<meta::value_category<Self&&>::value>::
        template with_idx<usize, internal::tuple::tuple_leaf>::template apply<
            I>(VEG_FWD(tup).m_impl);
  }

  template <typename Self, usize... Is>
  static constexpr auto HEDLEY_ALWAYS_INLINE
  as_ref_impl(Self&& self, meta::index_sequence<Is...> /*seq*/) noexcept
      -> tuple<decltype(tuple::ith_ref<Is>(VEG_FWD(self)))...> {
    return {tuple::ith_ref<Is>(VEG_FWD(self))...};
  }

  template <typename T>
  friend auto internal::tuple::get_inner(T&& tup) noexcept
      -> decltype((VEG_FWD(tup).m_impl))&&;
};

__VEG_CPP17(template <typename... Ts> tuple(Ts...) -> tuple<Ts...>;)

} // namespace veg

#endif /* end of include guard __VEG_TUPLE_GENERIC_HPP_DUSBI7AJS */
