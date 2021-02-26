#ifndef __VEG_TUPLE_GENERIC_HPP_DUSBI7AJS
#define __VEG_TUPLE_GENERIC_HPP_DUSBI7AJS

#include "veg/internal/type_traits.hpp"
#include "veg/internal/integer_seq.hpp"
#include "veg/internal/meta_int.hpp"
#include "veg/internal/storage.hpp"
#include <utility> // std::tuple_{size,element}

namespace veg {

template <typename... Ts>
struct tuple;

namespace internal {
namespace tuple {
struct hidden_tag1 {};
struct hidden_tag2 {};

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

template <usize I>
struct pack_ith_elem {
  template <typename... Ts>
  using type = decltype(
      storage::get_inner<meta::category_e::own>::
          template with_idx<usize, tuple_leaf>::template get_type<
              I>(__VEG_DECLVAL(
              tuple_impl<meta::make_index_sequence<sizeof...(Ts)>, Ts...>)));
};

template <usize... Is, typename... Ts>
struct tuple_impl<meta::index_sequence<Is...>, Ts...> : tuple_leaf<Is, Ts>... {
  constexpr tuple_impl() = default;
  HEDLEY_ALWAYS_INLINE constexpr explicit tuple_impl(Ts&&... args) noexcept(
      meta::all_of({meta::nothrow_constructible<Ts, Ts&&>::value...}))
      : tuple_leaf<Is, Ts>{storage::hidden_tag2{}, VEG_FWD(args)}... {}

  template <typename... Fn>
  HEDLEY_ALWAYS_INLINE constexpr explicit tuple_impl(
      hidden_tag1 /*unused*/,
      Fn&&... fns) noexcept(meta::all_of({meta::nothrow_invocable<Fn&&>::
                                              value...}))
      : tuple_leaf<Is, Ts>{storage::hidden_tag1{}, VEG_FWD(fns)}... {}

  template <typename... Us>
  HEDLEY_ALWAYS_INLINE constexpr explicit tuple_impl(
      hidden_tag2 /*unused*/,
      tuple_impl<meta::index_sequence<Is...>, Us...> const& tup)

      noexcept(
          meta::all_of({meta::nothrow_constructible<Ts, Us const&>::value...}))
      : tuple_leaf<Is, Ts>{
            storage::hidden_tag2{},
            static_cast<storage::storage<Us, false> const&>(
                static_cast<tuple_leaf<Is, Us> const&>(tup))
                ._get()}... {}

  template <typename... Us>
  HEDLEY_ALWAYS_INLINE constexpr explicit tuple_impl(
      hidden_tag2 /*unused*/,
      tuple_impl<meta::index_sequence<Is...>, Us...>&& tup)

      noexcept(meta::all_of({meta::nothrow_constructible<Ts, Us&&>::value...}))
      : tuple_leaf<Is, Ts>{
            storage::hidden_tag2{},
            static_cast<storage::storage<Us, false>&&>(
                static_cast<tuple_leaf<Is, Us>&&>(tup))
                .get_mov_ref()}... {}
};

template <>
struct tuple_impl<meta::index_sequence<>> {
  constexpr tuple_impl() = default;
};

template <bool Cvt>
struct assign_impl {
  template <typename L, typename R>
  HEDLEY_ALWAYS_INLINE static __VEG_CPP14(constexpr) void apply(
      L& l, R&& r) noexcept {
    l = static_cast<L>(VEG_FWD(r));
  }
};
template <>
struct assign_impl<false> {
  template <typename L, typename R>
  HEDLEY_ALWAYS_INLINE static __VEG_CPP14(constexpr) void apply(
      L&& l, R&& r) noexcept(meta::nothrow_assignable<L&&, R&&>::value) {
    VEG_FWD(l) = VEG_FWD(r);
  }
};

template <bool Do_Something>
struct cmp_impl {
  template <
      typename Cmp,
      usize Start = 0,
      usize... Is,
      typename... Ts,
      typename... Us>
  static HEDLEY_ALWAYS_INLINE constexpr auto apply(
      Cmp const* cmp,
      tuple_impl<meta::index_sequence<Is...>, Ts...> const* lhs,
      tuple_impl<meta::index_sequence<Is...>, Us...> const* rhs) noexcept
      -> bool {

#if __cplusplus >= 201703L
    return (
        (*cmp)(
            static_cast<storage::storage<Ts, false> const&>(
                static_cast<tuple_leaf<Is, Ts> const&>(*lhs))
                ._get(),

            static_cast<storage::storage<Us, false> const&>(
                static_cast<tuple_leaf<Is, Us> const&>(*rhs))
                ._get()) &&
        ...);
#else
    using T = typename pack_ith_elem<Start>::template type<Ts...>;
    using U = typename pack_ith_elem<Start>::template type<Us...>;

    return (*cmp)(
               static_cast<storage::storage<T, false> const&>(
                   static_cast<tuple_leaf<Start, T> const&>(*lhs))
                   ._get(),

               static_cast<storage::storage<U, false> const&>(
                   static_cast<tuple_leaf<Start, U> const&>(*rhs))
                   ._get()) &&

           cmp_impl<Start + 1 != sizeof...(Is)>::template apply<Start + 1>(
               lhs, rhs);
#endif
  }
};

template <>
struct cmp_impl<false> {
  template <usize Start = 0>
  static HEDLEY_ALWAYS_INLINE constexpr auto
  apply(void const* /*cmp*/, void const* /*lhs*/, void const* /*rhs*/) noexcept
      -> bool {
    return true;
  }
};

template <usize... Is, typename... Ts, typename... Us>
HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) void assign_(
    tuple_impl<meta::index_sequence<Is...>, Ts...>&& lhs,
    tuple_impl<meta::index_sequence<Is...>, Us...>&&
        rhs) noexcept(meta::all_of({meta::nothrow_assignable<Ts, Us>::
                                        value...})) {
  static_assert(meta::all_of({meta::reference<Ts>::value...}), "bug");
  static_assert(meta::all_of({meta::reference<Us>::value...}), "bug");

  (void)meta::internal::int_arr{
      0,
      (assign_impl<(                                            //
           meta::arithmetic<meta::remove_cvref_t<Ts>>::value && //
           meta::arithmetic<meta::remove_cvref_t<Us>>::value    //
           )>::
           apply(
               static_cast<storage::storage<Ts, false>&&>(
                   static_cast<tuple_leaf<Is, Ts>&&>(lhs))
                   .get_mov_ref(),

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
      (veg::swap(
           static_cast<storage::storage<Ts, false>&&>(
               static_cast<tuple_leaf<Is, Ts>&&>(lhs))
               .get_mov_ref(),

           static_cast<storage::storage<Us, false>&&>(
               static_cast<tuple_leaf<Is, Us>&&>(VEG_FWD(rhs)))
               .get_mov_ref()),
       0)...};
}

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
      (lhs, Tup_Lhs&&),
      (rhs, Tup_Rhs&&))
  noexcept(noexcept(internal::tuple::swap_( //
      VEG_FWD(lhs).as_ref().m_impl,
      VEG_FWD(rhs).as_ref().m_impl))) {
    return internal::tuple::swap_(
        VEG_FWD(lhs).as_ref().m_impl, VEG_FWD(rhs).as_ref().m_impl);
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

VEG_TEMPLATE(
    (typename... Ts, typename... Us),
    requires __VEG_ALL_OF((meta::equality_comparable_with<Ts, Us>::value)),
    HEDLEY_ALWAYS_INLINE constexpr auto
    operator==,
    (lhs, veg::tuple<Ts...> const&),
    (rhs, veg::tuple<Us...> const&))
noexcept -> bool {
  return cmp_impl<sizeof...(Ts) != 0>::apply(
      &cmp_equal,
      &storage::as_lvalue(
          veg::tuple<meta::remove_cvref_t<Ts> const&...>(lhs).m_impl),
      &storage::as_lvalue(
          veg::tuple<meta::remove_cvref_t<Us> const&...>(rhs).m_impl));
}
VEG_TEMPLATE(
    (typename... Ts, typename... Us),
    requires __VEG_ALL_OF((meta::equality_comparable_with<Ts, Us>::value)),
    HEDLEY_ALWAYS_INLINE constexpr auto
    operator!=,
    (lhs, veg::tuple<Ts...> const&),
    (rhs, veg::tuple<Us...> const&))
noexcept -> bool {
  return !cmp_impl<sizeof...(Ts) != 0>::apply(
      &cmp_equal,
      &storage::as_lvalue(
          veg::tuple<meta::remove_cvref_t<Ts> const&...>(lhs).m_impl),
      &storage::as_lvalue(
          veg::tuple<meta::remove_cvref_t<Us> const&...>(rhs).m_impl));
}

VEG_TEMPLATE(
    (typename... Ts, typename... Us),
    requires __VEG_ALL_OF((meta::partially_ordered_with<Ts, Us>::value)),
    HEDLEY_ALWAYS_INLINE constexpr auto
    operator<,
    (lhs, veg::tuple<Ts...> const&),
    (rhs, veg::tuple<Us...> const&))
noexcept -> bool {
  return cmp_impl<sizeof...(Ts) != 0>::apply(
      &cmp_less,
      &storage::as_lvalue(
          veg::tuple<meta::remove_cvref_t<Ts> const&...>(lhs).m_impl),
      &storage::as_lvalue(
          veg::tuple<meta::remove_cvref_t<Us> const&...>(rhs).m_impl));
}
VEG_TEMPLATE(
    (typename... Ts, typename... Us),
    requires __VEG_ALL_OF((meta::partially_ordered_with<Ts, Us>::value)),
    HEDLEY_ALWAYS_INLINE constexpr auto
    operator>=,
    (lhs, veg::tuple<Ts...> const&),
    (rhs, veg::tuple<Us...> const&))
noexcept -> bool {
  return !cmp_impl<sizeof...(Ts) != 0>::apply(
      &cmp_less,
      &storage::as_lvalue(
          veg::tuple<meta::remove_cvref_t<Ts> const&...>(lhs).m_impl),
      &storage::as_lvalue(
          veg::tuple<meta::remove_cvref_t<Us> const&...>(rhs).m_impl));
}

VEG_TEMPLATE(
    (typename... Ts, typename... Us),
    requires __VEG_ALL_OF((meta::partially_ordered_with<Us, Ts>::value)),
    HEDLEY_ALWAYS_INLINE constexpr auto
    operator>,
    (lhs, veg::tuple<Ts...> const&),
    (rhs, veg::tuple<Us...> const&))
noexcept -> bool {
  return cmp_impl<sizeof...(Ts) != 0>::apply(
      &cmp_less,
      &storage::as_lvalue(
          veg::tuple<meta::remove_cvref_t<Us> const&...>(rhs).m_impl),
      &storage::as_lvalue(
          veg::tuple<meta::remove_cvref_t<Ts> const&...>(lhs).m_impl));
}
VEG_TEMPLATE(
    (typename... Ts, typename... Us),
    requires __VEG_ALL_OF((meta::partially_ordered_with<Us, Ts>::value)),
    HEDLEY_ALWAYS_INLINE constexpr auto
    operator<=,
    (lhs, veg::tuple<Ts...> const&),
    (rhs, veg::tuple<Us...> const&))
noexcept -> bool {
  return !cmp_impl<sizeof...(Ts) != 0>::apply(
      &cmp_less,
      &storage::as_lvalue(
          veg::tuple<meta::remove_cvref_t<Us> const&...>(rhs).m_impl),
      &storage::as_lvalue(
          veg::tuple<meta::remove_cvref_t<Ts> const&...>(lhs).m_impl));
}

} // namespace adl

template <bool Movable, typename... Ts>
struct tuple_ctor_base : veg::internal::tuple::adl::tuple_base<Ts...> {
  constexpr tuple_ctor_base() = default;
  HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base(Ts... args) noexcept(
      meta::all_of({meta::nothrow_move_constructible<Ts>::value...}))
      : m_impl(VEG_FWD(args)...) {}

  VEG_TEMPLATE(
      (typename... Fns),
      requires __VEG_ALL_OF((
          meta::invocable<Fns&&>::value && //
          __VEG_SAME_AS(Ts, (meta::detected_t<meta::invoke_result_t, Fns&&>)))),
      HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base,
      (/*tag*/, inplace_t),
      (... fns, Fns&&))
  noexcept(meta::all_of({meta::nothrow_invocable<Fns&&>::value...}))
      : m_impl(internal::tuple::hidden_tag1{}, VEG_FWD(fns)...) {}

  VEG_TEMPLATE_EXPLICIT(
      !__VEG_ALL_OF(meta::convertible_to<Us&&, Ts>::value),
      (typename... Us),
      requires __VEG_ALL_OF(meta::constructible<Ts, Us&&>::value),
      HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base,
      ((tup, veg::tuple<Us...>&&)),
      noexcept(meta::all_of({meta::nothrow_constructible<Ts, Us&&>::value...}))
      : m_impl(hidden_tag2{}, VEG_FWD(tup).m_impl){})

  internal::tuple::tuple_impl<meta::make_index_sequence<sizeof...(Ts)>, Ts...>
      m_impl;
  __VEG_DEFAULTS(tuple_ctor_base);
};

template <typename... Ts>
struct tuple_ctor_base<false, Ts...>
    : veg::internal::tuple::adl::tuple_base<Ts...> {
  constexpr tuple_ctor_base() = default;

  VEG_TEMPLATE(
      (typename... Fns),
      requires __VEG_ALL_OF((
          meta::invocable<Fns&&>::value && //
          __VEG_SAME_AS(Ts, (meta::detected_t<meta::invoke_result_t, Fns&&>)))),
      HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base,
      (/*tag*/, inplace_t),
      (... fns, Fns&&))
  noexcept(meta::all_of({meta::nothrow_invocable<Fns&&>::value...}))
      : m_impl(internal::tuple::hidden_tag1{}, VEG_FWD(fns)...) {}

  VEG_TEMPLATE_EXPLICIT(
      !__VEG_ALL_OF(meta::convertible_to<Us&&, Ts>::value),
      (typename... Us),
      requires __VEG_ALL_OF(meta::constructible<Ts, Us&&>::value),
      HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base,
      ((tup, veg::tuple<Us...>&&)),
      noexcept(meta::all_of({meta::nothrow_constructible<Ts, Us&&>::value...}))
      : m_impl(hidden_tag2{}, VEG_FWD(tup).m_impl){})

  internal::tuple::tuple_impl<meta::make_index_sequence<sizeof...(Ts)>, Ts...>
      m_impl;
  __VEG_DEFAULTS(tuple_ctor_base);
};

template <>
struct tuple_ctor_base<true> : veg::internal::tuple::adl::tuple_base<> {
  constexpr tuple_ctor_base() = default;

  HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base /* NOLINT */ (
      inplace_t /*tag*/) noexcept
      : tuple_ctor_base{} {}

  internal::tuple::tuple_impl<meta::index_sequence<>> m_impl;
  __VEG_DEFAULTS(tuple_ctor_base);
};

template <bool Const_Self_Assign, typename... Ts>
struct tuple_assignment_base_copy {

  __VEG_CPP14(
      constexpr) // NOLINT(
                 // cppcoreguidelines-c-copy-assignment-signature,misc-unconventional-assign-operator)

  HEDLEY_ALWAYS_INLINE auto operator= // NOLINT(cert-oop54-cpp)
      (veg::tuple<Ts...> const& rhs) const& noexcept(
          meta::all_of({meta::assignable<Ts const&, Ts const&>::value...}))
          -> veg::tuple<Ts...> const& {
    auto const& self = static_cast<veg::tuple<Ts...> const&>(*this);
    tuple::assign_(self.as_ref().m_impl, rhs.as_ref().m_impl);
    return self;
  }
  tuple_assignment_base_copy() = default;
  __VEG_DEFAULTS(tuple_assignment_base_copy);
};
template <typename... Ts>
struct tuple_assignment_base_copy<false, Ts...> {
  tuple_assignment_base_copy() = default;
  __VEG_DEFAULTS(tuple_assignment_base_copy);
};

template <bool Const_Self_Assign, typename... Ts>
struct tuple_assignment_base_move {

  __VEG_CPP14(
      constexpr) // NOLINT(
                 // cppcoreguidelines-c-move-assignment-signature,misc-unconventional-assign-operator)

  HEDLEY_ALWAYS_INLINE auto operator= // NOLINT(cert-oop54-cpp)

      (veg::tuple<Ts...>&& rhs) const& noexcept(
          meta::all_of({meta::assignable<Ts const&, Ts&&>::value...}))
          -> veg::tuple<Ts...> const& {
    auto const& self = static_cast<veg::tuple<Ts...> const&>(*this);
    tuple::assign_(self.as_ref().m_impl, VEG_FWD(rhs).as_ref().m_impl);
    return self;
  }
  tuple_assignment_base_move() = default;
  __VEG_DEFAULTS(tuple_assignment_base_move);
};
template <typename... Ts>
struct tuple_assignment_base_move<false, Ts...> {
  tuple_assignment_base_move() = default;
  __VEG_DEFAULTS(tuple_assignment_base_move);
};

} // namespace tuple
} // namespace internal

template <typename... Ts>
struct tuple
    : internal::tuple::tuple_ctor_base<
          meta::all_of({meta::move_constructible<Ts>::value...}),
          Ts...>,
      internal::tuple::tuple_assignment_base_copy<
          meta::all_of({meta::assignable<Ts const&, Ts const&>::value...}),
          Ts...>,
      internal::tuple::tuple_assignment_base_move<
          meta::all_of({meta::assignable<Ts const&, Ts&&>::value...}),
          Ts...> {
  using ctor_base = internal::tuple::tuple_ctor_base<
      meta::all_of({meta::move_constructible<Ts>::value...}),
      Ts...>;
  using ctor_base::ctor_base;
  using internal::tuple::tuple_assignment_base_copy<
      meta::all_of({meta::assignable<Ts const&, Ts const&>::value...}),
      Ts...>::operator=;
  using internal::tuple::tuple_assignment_base_move<
      meta::all_of({meta::assignable<Ts const&, Ts&&>::value...}),
      Ts...>::operator=;

  __VEG_DEFAULTS(tuple);

  VEG_TEMPLATE(
      (typename... Us),
      requires !__VEG_ALL_OF(meta::constructible<Ts, Us&&>::value),
      HEDLEY_ALWAYS_INLINE constexpr tuple,
      (tup, veg::tuple<Us...>&&)) = delete;

  VEG_TEMPLATE_EXPLICIT(
      !__VEG_ALL_OF(meta::convertible_to<Us const&, Ts>::value),
      (typename... Us),
      requires __VEG_ALL_OF(meta::constructible<Ts, Us const&>::value),
      HEDLEY_ALWAYS_INLINE constexpr tuple,
      ((tup, veg::tuple<Us...> const&)),
      noexcept(
          meta::all_of({meta::nothrow_constructible<Ts, Us const&>::value...}))
      : ctor_base(tup.as_ref()){})

  VEG_TEMPLATE_EXPLICIT(
      !__VEG_ALL_OF(meta::convertible_to<Us&, Ts>::value),
      (typename... Us),
      requires __VEG_ALL_OF(meta::constructible<Ts, Us&>::value),
      HEDLEY_ALWAYS_INLINE constexpr tuple,
      ((tup, veg::tuple<Us...>&)),
      noexcept(meta::all_of({meta::nothrow_constructible<Ts, Us&>::value...}))
      : ctor_base(tup.as_ref()){})

  __VEG_CVREF_DUPLICATE(
      VEG_TEMPLATE(
          (i64 I),
          requires(
              I < sizeof...(Ts) && (I >= 0) &&
              meta::constructible<
                  typename internal::tuple::pack_ith_elem<I>::template type<
                      Ts...>,
                  typename internal::tuple::pack_ith_elem<I>::template type<
                      Ts...>&&>::value),
          HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto
          operator[],
          (/*arg*/, fix<I>)),
      internal::tuple::adl_get<I>,
      ());
  VEG_TEMPLATE(
      (i64 I),
      requires(
          I < sizeof...(Ts) && (I >= 0) &&
          !meta::constructible<
              typename internal::tuple::pack_ith_elem<I>::template type<Ts...>,
              typename internal::tuple::pack_ith_elem<I>::template type<
                  Ts...>&&>::value),
      HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto
      operator[],
      (/*arg*/, fix<I>)) = delete;

  VEG_TEMPLATE(
      (typename... Us),
      requires __VEG_ALL_OF(
          (meta::reference<Ts>::value &&
           meta::assignable<Ts&, Us const&>::value)),
      auto
      operator=,
      (rhs, veg::tuple<Us...> const&)) & noexcept -> void = delete;
  VEG_TEMPLATE(
      (typename... Us),
      requires __VEG_ALL_OF(
          (meta::reference<Ts>::value && meta::assignable<Ts&, Us&&>::value)),
      auto
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
    internal::tuple::assign_(this->as_ref().m_impl, rhs.as_ref().m_impl);
    return *this;
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
    internal::tuple::assign_(this->as_ref().m_impl, rhs.as_ref().m_impl);
    return *this;
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
    internal::tuple::assign_(this->as_ref().m_impl, rhs.as_ref().m_impl);
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
    internal::tuple::assign_(this->as_ref().m_impl, rhs.as_ref().m_impl);
    return static_cast<veg::tuple<Ts...> const&>(*this);
  }

  __VEG_CVREF_DUPLICATE(
      HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto as_ref(),
      tuple::as_ref_impl,
      (, meta::make_index_sequence<sizeof...(Ts)>{}));

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
      -> veg::tuple<decltype(tuple::ith_ref<Is>(VEG_FWD(self)))...> {
    return veg::tuple<decltype(tuple::ith_ref<Is>(VEG_FWD(self)))...>{
        tuple::ith_ref<Is>(VEG_FWD(self))...};
  }

  template <typename T>
  friend auto constexpr internal::tuple::get_inner(T&& tup) noexcept
      -> decltype((VEG_FWD(tup).m_impl))&&;

  friend struct internal::tuple::impl;
};

__VEG_CPP17(template <typename... Ts> tuple(Ts...) -> tuple<Ts...>;)

} // namespace veg

#endif /* end of include guard __VEG_TUPLE_GENERIC_HPP_DUSBI7AJS */
