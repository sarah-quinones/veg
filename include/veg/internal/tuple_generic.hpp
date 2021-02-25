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
struct tuple_leaf : storage::storage<T, false> {
  using storage::storage<T, false>::storage;
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

VEG_TEMPLATE(
    (usize... Is, typename... Ts, typename... Us),
    requires __VEG_ALL_OF(meta::assignable<Ts, Us>::value),
    HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) void assign_,

    (lhs, tuple_impl<meta::index_sequence<Is...>, Ts...>&&),
    (rhs, tuple_impl<meta::index_sequence<Is...>, Us...>&&))

noexcept(meta::all_of({meta::nothrow_assignable<Ts, Us>::value...})) {
  static_assert(meta::all_of({meta::reference<Ts>::value...}), "bug");
  static_assert(meta::all_of({meta::reference<Us>::value...}), "bug");

  (void)meta::internal::int_arr{
      0,
      (static_cast<storage::storage<Ts, false>&&>(
           static_cast<tuple_leaf<Is, Ts>&&>(lhs))
           .assign( //
               static_cast<storage::storage<Us, false>&&>(
                   static_cast<tuple_leaf<Is, Us>&&>(VEG_FWD(rhs)))
                   .get_mov_ref()),
       0)...};
}

VEG_TEMPLATE(
    (usize... Is, typename... Ts, typename... Us),
    requires __VEG_ALL_OF(meta::swappable<Ts, Us>::value),
    HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) void swap_,

    (lhs, tuple_impl<meta::index_sequence<Is...>, Ts...>&&),
    (rhs, tuple_impl<meta::index_sequence<Is...>, Us...>&&))

noexcept(meta::all_of({meta::nothrow_swappable<Ts, Us>::value...})) {
  static_assert(meta::all_of({meta::reference<Ts>::value...}), "bug");
  static_assert(meta::all_of({meta::reference<Us>::value...}), "bug");

  (void)meta::internal::int_arr{
      0,
      (static_cast<storage::storage<Ts, false>&&>(
           static_cast<tuple_leaf<Is, Ts>&&>(lhs))
           .swap( //
               static_cast<storage::storage<Us, false>&&>(
                   static_cast<tuple_leaf<Is, Us>&&>(VEG_FWD(rhs)))
                   .get_mov_ref()),
       0)...};
}

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
struct is_tuple : meta::false_type {};
template <typename... Ts>
struct is_tuple<veg::tuple<Ts...>> : meta::true_type {};

template <typename T>
struct mut_tuple_has_references : meta::false_type {};
template <typename... Ts>
struct mut_tuple_has_references<veg::tuple<Ts...>&>
    : meta::bool_constant<meta::all_of({meta::reference<Ts>::value...})> {};

template <typename T>
HEDLEY_ALWAYS_INLINE constexpr auto get_inner(T&& tup) noexcept
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

struct impl {
  VEG_TEMPLATE(
      (typename Tup_Lhs, typename Tup_Rhs),
      requires                                            //
      ((is_tuple<meta::remove_cvref_t<Tup_Lhs>>::value && //
        is_tuple<meta::remove_cvref_t<Tup_Rhs>>::value && //
        !mut_tuple_has_references<Tup_Lhs>::value &&
        !mut_tuple_has_references<Tup_Rhs>::value &&
        __VEG_SAME_AS(
            void,
            decltype(internal::tuple::swap_( //
                __VEG_DECLVAL(Tup_Lhs).as_ref().m_impl,
                __VEG_DECLVAL(Tup_Rhs).as_ref().m_impl))))),
      static HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) void swap,
      (ts, Tup_Lhs&&),
      (us, Tup_Rhs&&))
  noexcept(noexcept(internal::tuple::swap_( //
      __VEG_DECLVAL_NOEXCEPT(Tup_Lhs).as_ref().m_impl,
      __VEG_DECLVAL_NOEXCEPT(Tup_Rhs).as_ref().m_impl))) {
    return internal::tuple::swap_(
        VEG_FWD(ts).as_ref().m_impl, VEG_FWD(us).as_ref().m_impl);
  }
};

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
    (typename Tup_Lhs, typename Tup_Rhs),
    requires //
    __VEG_SAME_AS(
        void,
        decltype(internal::tuple::impl::swap( //
            __VEG_DECLVAL(Tup_Lhs),
            __VEG_DECLVAL(Tup_Rhs)))),
    HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) void swap,
    (ts, Tup_Lhs&&),
    (us, Tup_Rhs&&))
noexcept(noexcept(internal::tuple::impl::swap( //
    __VEG_DECLVAL_NOEXCEPT(Tup_Lhs),
    __VEG_DECLVAL_NOEXCEPT(Tup_Rhs)))) {
  return internal::tuple::impl::swap(VEG_FWD(ts), VEG_FWD(us));
}

} // namespace adl

template <typename... Ts>
struct tuple_interface : veg::internal::tuple::adl::tuple_base<Ts...> {
  constexpr tuple_interface() = default;
  HEDLEY_ALWAYS_INLINE constexpr tuple_interface /* NOLINT(hicpp-explicit-conversions)
                                                  */
      (Ts... args) noexcept(
          meta::all_of({meta::nothrow_move_constructible<Ts>::value...}))
      : m_impl(VEG_FWD(args)...) {}

  VEG_TEMPLATE(
      (typename... Fns),
      requires __VEG_ALL_OF((
          meta::invocable<Fns&&>::value && //
          __VEG_SAME_AS(Ts, (meta::detected_t<meta::invoke_result_t, Fns&&>)))),
      HEDLEY_ALWAYS_INLINE constexpr tuple_interface // NOLINT(hicpp-explicit-conversions)
      ,
      (/*tag*/, inplace_t),
      (... fns, Fns&&))
  noexcept
      : m_impl(internal::tuple::hidden_tag{}, VEG_FWD(fns)...) {}

  __VEG_CVREF_DUPLICATE(
      HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto as_ref(),
      tuple_interface::as_ref_impl,
      (, meta::make_index_sequence<sizeof...(Ts)>{}));

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
      -> veg::tuple<decltype(tuple_interface::ith_ref<Is>(VEG_FWD(self)))...> {
    return {tuple_interface::ith_ref<Is>(VEG_FWD(self))...};
  }

  template <typename T>
  friend auto constexpr internal::tuple::get_inner(T&& tup) noexcept
      -> decltype((VEG_FWD(tup).m_impl))&&;

  friend struct internal::tuple::impl;
};

template <bool Const_Self_Assign, typename... Ts>
struct tuple_assignment_base_copy : internal::tuple::tuple_interface<Ts...> {
  using internal::tuple::tuple_interface<Ts...>::tuple_interface;
  using internal::tuple::tuple_interface<Ts...>::operator=;

  __VEG_CPP14(
      constexpr) // NOLINT(
                 // cppcoreguidelines-c-copy-assignment-signature,misc-unconventional-assign-operator)

  HEDLEY_ALWAYS_INLINE auto operator= // NOLINT(cert-oop54-cpp)
      (veg::tuple<Ts...> const& rhs) const& noexcept(
          meta::all_of({meta::assignable<Ts const&, Ts const&>::value...}))
          -> veg::tuple<Ts...> const& {
    tuple::assign_(this->as_ref().m_impl, rhs.as_ref().m_impl);
    return static_cast<veg::tuple<Ts...> const&>(*this);
  }
};
template <typename... Ts>
struct tuple_assignment_base_copy<false, Ts...>
    : internal::tuple::tuple_interface<Ts...> {
  using internal::tuple::tuple_interface<Ts...>::tuple_interface;
  using internal::tuple::tuple_interface<Ts...>::operator=;
};

template <bool Const_Self_Assign, bool Copy, typename... Ts>
struct tuple_assignment_base_move
    : internal::tuple::tuple_assignment_base_copy<Copy, Ts...> {
  using internal::tuple::tuple_assignment_base_copy<Copy, Ts...>::
      tuple_assignment_base_copy;
  using internal::tuple::tuple_assignment_base_copy<Copy, Ts...>::operator=;

  __VEG_CPP14(
      constexpr) // NOLINT(
                 // cppcoreguidelines-c-move-assignment-signature,misc-unconventional-assign-operator)

  HEDLEY_ALWAYS_INLINE auto operator= // NOLINT(cert-oop54-cpp)

      (veg::tuple<Ts...>&& rhs) const& noexcept(
          meta::all_of({meta::assignable<Ts const&, Ts&&>::value...}))
          -> veg::tuple<Ts...> const& {
    tuple::assign_(this->as_ref().m_impl, rhs.as_ref().m_impl);
    return static_cast<veg::tuple<Ts...> const&>(*this);
  }
};
template <bool Copy, typename... Ts>
struct tuple_assignment_base_move<false, Copy, Ts...>
    : internal::tuple::tuple_assignment_base_copy<Copy, Ts...> {
  using internal::tuple::tuple_assignment_base_copy<Copy, Ts...>::
      tuple_assignment_base_copy;
  using internal::tuple::tuple_assignment_base_copy<Copy, Ts...>::operator=;
};

template <typename... Ts>
struct tuple_assignment_base
    : internal::tuple::tuple_assignment_base_move<
          meta::all_of({meta::assignable<Ts const&, Ts&&>::value...}),
          meta::all_of({meta::assignable<Ts const&, Ts const&>::value...}),
          Ts...> {
  using internal::tuple::tuple_assignment_base_move<
      meta::all_of({meta::assignable<Ts const&, Ts&&>::value...}),
      meta::all_of({meta::assignable<Ts const&, Ts const&>::value...}),
      Ts...>::tuple_assignment_base_move;
  using internal::tuple::tuple_assignment_base_move<
      meta::all_of({meta::assignable<Ts const&, Ts&&>::value...}),
      meta::all_of({meta::assignable<Ts const&, Ts const&>::value...}),
      Ts...>::operator=;

  VEG_TEMPLATE(
      (typename... Us),
      requires __VEG_ALL_OF(
          (meta::reference<Ts>::value &&
           meta::assignable<Ts&, Us const&>::value)),
      HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto
      operator=,
      (rhs, veg::tuple<Us...> const&)) & noexcept -> void = delete;
  VEG_TEMPLATE(
      (typename... Us),
      requires __VEG_ALL_OF(
          (meta::reference<Ts>::value && meta::assignable<Ts&, Us&&>::value)),
      HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto
      operator=,
      (rhs, veg::tuple<Us...>&&)) & noexcept -> void = delete;

  VEG_TEMPLATE(
      (typename... Us),
      requires __VEG_ALL_OF(
          (!meta::reference<Ts>::value &&
           meta::assignable<Ts&, Us const&>::value)),
      HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto
      operator=,
      (rhs, veg::tuple<Us...> const&)) &

      noexcept(
          meta::all_of({meta::nothrow_assignable<Ts&, Us const&>::value...}))
          -> veg::tuple<Ts...>& {
    tuple::assign_(this->as_ref().m_impl, rhs.as_ref().m_impl);
    return static_cast<veg::tuple<Ts...>&>(*this);
  }

  VEG_TEMPLATE(
      (typename... Us),
      requires __VEG_ALL_OF(
          (!meta::reference<Ts>::value && meta::assignable<Ts&, Us&&>::value)),
      HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto
      operator=,
      (rhs, veg::tuple<Us...>&&)) &

      noexcept(meta::all_of({meta::nothrow_assignable<Ts&, Us&&>::value...}))
          -> veg::tuple<Ts...>& {
    tuple::assign_(this->as_ref().m_impl, rhs.as_ref().m_impl);
    return static_cast<veg::tuple<Ts...>&>(*this);
  }

  VEG_TEMPLATE(
      (typename... Us),
      requires __VEG_ALL_OF(meta::assignable<Ts const&, Us const&>::value),
      HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto
      operator=,
      (rhs, veg::tuple<Us...> const&))
  const&

      noexcept(meta::all_of(
                   {meta::nothrow_assignable<Ts const&, Us const&>::value...}))
          ->veg::tuple<Ts...> const& {
    tuple::assign_(this->as_ref().m_impl, rhs.as_ref().m_impl);
    return static_cast<veg::tuple<Ts...> const&>(*this);
  }

  VEG_TEMPLATE(
      (typename... Us),
      requires __VEG_ALL_OF(meta::assignable<Ts const&, Us&&>::value),
      HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto
      operator=,
      (rhs, veg::tuple<Us...>&&))
  const&

      noexcept(
          meta::all_of({meta::nothrow_assignable<Ts const&, Us&&>::value...}))
          ->veg::tuple<Ts...> const& {
    tuple::assign_(this->as_ref().m_impl, rhs.as_ref().m_impl);
    return static_cast<veg::tuple<Ts...> const&>(*this);
  }
};

} // namespace tuple
} // namespace internal

template <typename... Ts>
struct tuple : internal::tuple::tuple_assignment_base<Ts...> {
  using internal::tuple::tuple_assignment_base<Ts...>::tuple_assignment_base;
  using internal::tuple::tuple_assignment_base<Ts...>::operator=;

  __VEG_CVREF_DUPLICATE(
      VEG_TEMPLATE(
          (i64 I),
          requires(I < sizeof...(Ts) && (I >= 0)),
          HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto
          operator[],
          (/*arg*/, fix<I>)),
      internal::tuple::adl_get<I>,
      ());
};

__VEG_CPP17(template <typename... Ts> tuple(Ts...) -> tuple<Ts...>;)

} // namespace veg

#endif /* end of include guard __VEG_TUPLE_GENERIC_HPP_DUSBI7AJS */
