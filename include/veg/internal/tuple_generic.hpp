#ifndef __VEG_TUPLE_GENERIC_HPP_DUSBI7AJS
#define __VEG_TUPLE_GENERIC_HPP_DUSBI7AJS

#include "veg/internal/type_traits.hpp"
#include "veg/internal/integer_seq.hpp"
#include "veg/internal/meta_int_fix.hpp"
#include "veg/internal/storage.hpp"
#include "veg/internal/cmp.hpp"
#include <utility> // std::tuple_{size,element}

namespace veg {

template <typename... Ts>
struct tuple;

namespace internal {
namespace tuple {

template <typename T, typename U>
struct trivially_swappable : meta::false_type {};
template <typename... Ts>
struct trivially_swappable<veg::tuple<Ts...>&, veg::tuple<Ts...>&>
    : meta::bool_constant<meta::all_of(
          {!__VEG_CONCEPT(meta::reference<Ts>) &&
           trivially_swappable<Ts&, Ts&>::value...})> {};
template <typename... Ts>
struct trivially_swappable<veg::tuple<Ts...> const&, veg::tuple<Ts...> const&>
    : meta::false_type {};

} // namespace tuple
} // namespace internal

namespace meta {
template <typename... Ts>
struct is_trivially_swappable<tuple<Ts...>&>
    : veg::internal::tuple::trivially_swappable<tuple<Ts...>&, tuple<Ts...>&> {
};
template <typename... Ts>
struct is_trivially_swappable<tuple<Ts...> const&> : false_type {};
} // namespace meta

namespace internal {
namespace tuple {

struct hidden_tag0 {};
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
  HEDLEY_ALWAYS_INLINE constexpr explicit tuple_impl(
      hidden_tag0 /*unused*/, Ts&&... args)

      noexcept(meta::all_of(
          {__VEG_CONCEPT(meta::nothrow_constructible<Ts, Ts&&>)...}))
      : tuple_leaf<Is, Ts>{storage::hidden_tag2{}, VEG_FWD(args)}... {}

  template <typename... Fn>
  HEDLEY_ALWAYS_INLINE constexpr explicit tuple_impl(
      hidden_tag1 /*unused*/,
      Fn&&... fns) noexcept(meta::
                                all_of({__VEG_CONCEPT(
                                    meta::nothrow_invocable<Fn&&>)...}))
      : tuple_leaf<Is, Ts>{storage::hidden_tag1{}, VEG_FWD(fns)}... {}

  template <meta::category_e C, typename... Us>
  HEDLEY_ALWAYS_INLINE constexpr explicit tuple_impl(
      hidden_tag2 /*unused*/,
      std::integral_constant<meta::category_e, C>* /*unused*/,
      tuple_impl<meta::index_sequence<Is...>, Us...> const& tup)

      noexcept(meta::all_of({__VEG_CONCEPT(
          meta::
              nothrow_constructible<Ts, meta::apply_category_t<C, Us>&&>)...}))
      : tuple_leaf<Is, Ts>{
            storage::hidden_tag2{},
            const_cast<meta::apply_category_t<C, Us>&&> /* NOLINT */ (
                static_cast<tuple_leaf<Is, Us> const&>(tup).inner)}... {}
};

template <typename... Ts>
struct binop_ftor {
  template <typename Fn, bool NoExcept, typename... Us>
  static __VEG_CPP14(constexpr) void apply(Ts&&... ts, Us&&... us) noexcept(
      NoExcept) {
    (void)meta::internal::int_arr{0, (Fn::apply(ts, us), 0)...};
  }
};

template <
    typename Fn,
    bool NoExcept,
    typename... Actual_Ts,
    typename... Actual_Us,
    usize... Is,
    typename... Ts,
    typename... Us>
HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) void binop_apply(
    veg::tuple<Actual_Ts...>* /*unused*/,
    veg::tuple<Actual_Us...>* /*unused*/,
    tuple_impl<meta::index_sequence<Is...>, Ts...> const& t,
    tuple_impl<meta::index_sequence<Is...>, Us...> const& u)

    noexcept(NoExcept) {
  binop_ftor<Actual_Ts&&...>::template apply<Fn, NoExcept>(

      const_cast<Actual_Ts&&> /* NOLINT */ (
          static_cast<tuple_leaf<Is, Ts> const&>(t).inner)...,
      const_cast<Actual_Us&&> /* NOLINT */ (
          static_cast<tuple_leaf<Is, Us> const&>(u).inner)...

  );
}

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
      L&& l,
      R&& r) noexcept(__VEG_CONCEPT(meta::nothrow_assignable<L&&, R&&>)) {
    VEG_FWD(l) = VEG_FWD(r);
  }
};

struct assign_ftor {
  template <typename L, typename R>
  HEDLEY_ALWAYS_INLINE static __VEG_CPP14(constexpr) void apply(
      L&& l,
      R&& r) noexcept(__VEG_CONCEPT(meta::nothrow_assignable<L&&, R&&>)) {
    assign_impl<
        __VEG_CONCEPT(meta::arithmetic<meta::remove_cvref_t<L>>) &&
        __VEG_CONCEPT(meta::arithmetic<meta::remove_cvref_t<R>>)>::
        apply(VEG_FWD(l), VEG_FWD(r));
  }
};
struct swap_ftor {
  template <typename L, typename R>
  HEDLEY_ALWAYS_INLINE static __VEG_CPP14(constexpr) void apply(
      L&& l, R&& r) noexcept(__VEG_CONCEPT(meta::nothrow_swappable<L&&, R&&>)) {
    veg::swap(VEG_FWD(l), VEG_FWD(r));
  }
};

#if __cplusplus >= 201703L
template <typename... Ts>
struct cmp_eq_impl {
  template <typename... Us>
  HEDLEY_ALWAYS_INLINE static constexpr auto
  apply(Ts const&... ts, Us const&... us) noexcept -> bool {
    return (veg::cmp_equal(ts, us) && ... && true);
  }
};
#else
template <typename... Ts>
struct cmp_eq_impl;
template <typename T0, typename... Ts>
struct cmp_eq_impl<T0, Ts...> {
  template <typename U0, typename... Us>
  HEDLEY_ALWAYS_INLINE static constexpr auto
  apply(T0 const& t0, Ts const&... ts, U0 const& u0, Us const&... us) noexcept
      -> bool {
    return (veg::cmp_equal(t0, u0) && cmp_eq_impl<Ts...>::apply(ts..., us...));
  }
};
template <>
struct cmp_eq_impl<> {
  HEDLEY_ALWAYS_INLINE static constexpr auto apply() noexcept -> bool {
    return true;
  }
};
#endif
// TODO: three-way comparison customization point

struct cmp_impl {
  template <usize... Is, typename... Ts, typename... Us>
  static HEDLEY_ALWAYS_INLINE constexpr auto apply(
      tuple_impl<meta::index_sequence<Is...>, Ts...> const& lhs,
      tuple_impl<meta::index_sequence<Is...>, Us...> const& rhs) noexcept
      -> bool {

    return cmp_eq_impl<meta::remove_cvref_t<Ts>...>::apply(

        static_cast<tuple_leaf<Is, Ts> const&>(lhs).inner...,
        static_cast<tuple_leaf<Is, Us> const&>(rhs).inner...

    );
  }
};

template <
    bool NoExcept,
    meta::category_e CL,
    meta::category_e CR,
    usize... Is,
    typename... Ts,
    typename... Us>
HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) void swap_impl(
    tuple_impl<meta::index_sequence<Is...>, Ts...> const& lhs,
    tuple_impl<meta::index_sequence<Is...>, Us...> const& rhs)

    noexcept(NoExcept) {

  tuple::binop_apply<swap_ftor, NoExcept>(
      static_cast<veg::tuple<meta::apply_category_t<CL, Ts>&&...>*>(nullptr),
      static_cast<veg::tuple<meta::apply_category_t<CR, Us>&&...>*>(nullptr),
      lhs,
      rhs);
}

template <typename T>
struct is_tuple : meta::false_type {};
template <typename... Ts>
struct is_tuple<veg::tuple<Ts...>> : meta::true_type {};

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

template <bool Trivial>
struct impl {
  template <bool, typename Tup>
  static HEDLEY_ALWAYS_INLINE
  __VEG_CPP14(constexpr) void apply(Tup& ts, Tup& us) noexcept {
    return internal::swap_::mov_fn_swap::apply(ts, us);
  }
};
template <>
struct impl<false> {
  template <bool NoExcept, typename Tup_Lhs, typename Tup_Rhs>
  static HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) void apply(
      Tup_Lhs&& lhs, Tup_Rhs&& rhs) noexcept(NoExcept) {
    return internal::tuple::swap_impl<
        NoExcept,
        meta::value_category<Tup_Lhs&&>::value,
        meta::value_category<Tup_Rhs&&>::value>(
        tuple::get_inner(lhs), tuple::get_inner(rhs));
  }
};

template <
    typename Tup_L,
    typename Tup_R,
    meta::category_e CL,
    meta::category_e CR>
struct tup_swappable : meta::false_type {};

template <
    typename... Ts,
    typename... Us,
    meta::category_e CL,
    meta::category_e CR>
struct tup_swappable<veg::tuple<Ts...>, veg::tuple<Us...>, CL, CR>
    : meta::bool_constant<meta::all_of(
          {__VEG_CONCEPT(meta::swappable<                //
                         meta::apply_category_t<CL, Ts>, //
                         meta::apply_category_t<CR, Us>  //
                         >)...})> {};

template <
    typename Tup_L,
    typename Tup_R,
    meta::category_e CL,
    meta::category_e CR>
struct nothrow_tup_swappable : meta::false_type {};

template <
    typename... Ts,
    typename... Us,
    meta::category_e CL,
    meta::category_e CR>
struct nothrow_tup_swappable<veg::tuple<Ts...>, veg::tuple<Us...>, CL, CR>
    : meta::bool_constant<meta::all_of(
          {__VEG_CONCEPT(meta::nothrow_swappable<        //
                         meta::apply_category_t<CL, Ts>, //
                         meta::apply_category_t<CR, Us>  //
                         >)...})> {};

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
    (typename Tup_Lhs, typename Tup_Rhs, int = 0),
    requires(__VEG_CONCEPT(__VEG_DISJUNCTION(                             //
        (__VEG_TO_CONCEPT(tuple::trivially_swappable<Tup_Lhs, Tup_Rhs>)), //
        (__VEG_TO_CONCEPT(tup_swappable<                                  //
                          meta::remove_cvref_t<Tup_Lhs>,                  //
                          meta::remove_cvref_t<Tup_Rhs>,                  //
                          meta::value_category<Tup_Lhs>::value,           //
                          meta::value_category<Tup_Rhs>::value>))         //
        ))),
    HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) void swap,
    (ts, Tup_Lhs&&),
    (us, Tup_Rhs&&))
noexcept(meta::disjunction<
         meta::is_trivially_swappable<Tup_Lhs>,
         nothrow_tup_swappable<
             meta::remove_cvref_t<Tup_Lhs>,
             meta::remove_cvref_t<Tup_Rhs>,
             meta::value_category<Tup_Lhs>::value,
             meta::value_category<Tup_Rhs>::value>>::value) {
  return internal::tuple::
      impl<tuple::trivially_swappable<Tup_Lhs, Tup_Rhs>::value>::template apply<
          meta::disjunction<
              meta::is_trivially_swappable<Tup_Lhs>,
              nothrow_tup_swappable<
                  meta::remove_cvref_t<Tup_Lhs>,
                  meta::remove_cvref_t<Tup_Rhs>,
                  meta::value_category<Tup_Lhs>::value,
                  meta::value_category<Tup_Rhs>::value>>::value>(
          VEG_FWD(ts), VEG_FWD(us));
}

VEG_TEMPLATE(
    (typename... Ts, typename... Us),
    requires __VEG_ALL_OF(
        __VEG_CONCEPT(meta::equality_comparable_with<Ts, Us>)),
    HEDLEY_ALWAYS_INLINE constexpr auto
    operator==,
    (lhs, veg::tuple<Ts...> const&),
    (rhs, veg::tuple<Us...> const&))
noexcept -> bool {
  return cmp_impl::apply(tuple::get_inner(lhs), tuple::get_inner(rhs));
}
VEG_TEMPLATE(
    (typename... Ts, typename... Us),
    requires __VEG_ALL_OF(
        __VEG_CONCEPT(meta::equality_comparable_with<Ts, Us>)),
    HEDLEY_ALWAYS_INLINE constexpr auto
    operator!=,
    (lhs, veg::tuple<Ts...> const&),
    (rhs, veg::tuple<Us...> const&))
noexcept -> bool {
  return !cmp_impl::apply(tuple::get_inner(lhs), tuple::get_inner(rhs));
}

} // namespace adl

template <bool Movable, typename... Ts>
struct tuple_ctor_base : veg::internal::tuple::adl::tuple_base<Ts...> {
  constexpr tuple_ctor_base() = default;

  HEDLEY_ALWAYS_INLINE constexpr explicit tuple_ctor_base(Ts... args) noexcept(
      meta::all_of({__VEG_CONCEPT(meta::nothrow_move_constructible<Ts>)...}))
      : m_impl(hidden_tag0{}, VEG_FWD(args)...) {}

  HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base /* NOLINT */
      (elems_t /*tag*/, Ts... args) noexcept(meta::all_of(
          {__VEG_CONCEPT(meta::nothrow_move_constructible<Ts>)...}))
      : m_impl(hidden_tag0{}, VEG_FWD(args)...) {}

  VEG_TEMPLATE(
      (typename... Fns),
      requires __VEG_ALL_OF(
          __VEG_CONCEPT(meta::invocable<Fns&&>) && //
          __VEG_SAME_AS(Ts, (meta::detected_t<meta::invoke_result_t, Fns&&>))),
      HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base,
      (/*tag*/, inplace_t),
      (... fns, Fns&&))
  noexcept(meta::all_of({__VEG_CONCEPT(meta::nothrow_invocable<Fns&&>)...}))
      : m_impl(internal::tuple::hidden_tag1{}, VEG_FWD(fns)...) {}

  VEG_TEMPLATE_EXPLICIT(
      !__VEG_ALL_OF(__VEG_CONCEPT(meta::convertible_to<Us&&, Ts>)),
      (typename... Us),
      requires __VEG_ALL_OF(__VEG_CONCEPT(meta::constructible<Ts, Us&&>)),
      HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base,
      ((tup, veg::tuple<Us...>&&)),
      noexcept(meta::all_of(
          {__VEG_CONCEPT(meta::nothrow_constructible<Ts, Us&&>)...}))
      : m_impl(
          hidden_tag2{},
          static_cast<std::integral_constant<
              meta::category_e,
              meta::category_e::ref_mov>*>(nullptr),
          tuple::get_inner(VEG_FWD(tup))){})

  VEG_TEMPLATE_EXPLICIT(
      !__VEG_ALL_OF(__VEG_CONCEPT(meta::convertible_to<Us&, Ts>)),
      (typename... Us),
      requires __VEG_ALL_OF(
          (__VEG_CONCEPT(meta::constructible<Ts, Us&>) &&
           !__VEG_CONCEPT(meta::constructible<Ts, Us const&>))),
      HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base,
      ((tup, veg::tuple<Us...>&)),
      noexcept(meta::all_of(
          {__VEG_CONCEPT(meta::nothrow_constructible<Ts, Us&>)...}))
      : m_impl(
          hidden_tag2{},
          static_cast<std::integral_constant<
              meta::category_e,
              meta::category_e::ref_mut>*>(nullptr),
          tuple::get_inner(VEG_FWD(tup))){})

  VEG_TEMPLATE_EXPLICIT(
      !__VEG_ALL_OF(__VEG_CONCEPT(meta::convertible_to<Us const&, Ts>)),
      (typename... Us),
      requires __VEG_ALL_OF(__VEG_CONCEPT(meta::constructible<Ts, Us const&>)),
      HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base,
      ((tup, veg::tuple<Us...> const&)),
      noexcept(meta::all_of(
          {__VEG_CONCEPT(meta::nothrow_constructible<Ts, Us const&>)...}))
      : m_impl(
          hidden_tag2{},
          static_cast<
              std::integral_constant<meta::category_e, meta::category_e::ref>*>(
              nullptr),
          tuple::get_inner(VEG_FWD(tup))){})

  __VEG_DEFAULTS(tuple_ctor_base);

private:
  internal::tuple::tuple_impl<meta::make_index_sequence<sizeof...(Ts)>, Ts...>
      m_impl;

  template <typename T>
  friend auto constexpr internal::tuple::get_inner(T&& tup) noexcept
      -> decltype((VEG_FWD(tup).m_impl))&&;
};

template <>
struct tuple_ctor_base<true> : veg::internal::tuple::adl::tuple_base<> {
  constexpr tuple_ctor_base() = default;

  HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base /* NOLINT */
      (elems_t /*tag*/) noexcept
      : tuple_ctor_base() {}
  HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base /* NOLINT */
      (inplace_t /*tag*/) noexcept
      : tuple_ctor_base() {}

  __VEG_DEFAULTS(tuple_ctor_base);

private:
  internal::tuple::tuple_impl<meta::index_sequence<>> m_impl;

  template <typename T>
  friend auto constexpr internal::tuple::get_inner(T&& tup) noexcept
      -> decltype((VEG_FWD(tup).m_impl))&&;
};

template <typename... Ts>
struct tuple_ctor_base<false, Ts...>
    : veg::internal::tuple::adl::tuple_base<Ts...> {
  constexpr tuple_ctor_base() = default;

  VEG_TEMPLATE(
      (typename... Fns),
      requires __VEG_ALL_OF((
          __VEG_CONCEPT(meta::invocable<Fns&&>) && //
          __VEG_SAME_AS(Ts, (meta::detected_t<meta::invoke_result_t, Fns&&>)))),
      HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base,
      (/*tag*/, inplace_t),
      (... fns, Fns&&))
  noexcept(meta::all_of({__VEG_CONCEPT(meta::nothrow_invocable<Fns&&>)...}))
      : m_impl(internal::tuple::hidden_tag1{}, VEG_FWD(fns)...) {}

  VEG_TEMPLATE_EXPLICIT(
      !__VEG_ALL_OF(__VEG_CONCEPT(meta::convertible_to<Us&&, Ts>)),
      (typename... Us),
      requires __VEG_ALL_OF(__VEG_CONCEPT(meta::constructible<Ts, Us&&>)),
      HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base,
      ((tup, veg::tuple<Us...>&&)),
      noexcept(meta::all_of(
          {__VEG_CONCEPT(meta::nothrow_constructible<Ts, Us&&>)...}))
      : m_impl(
          hidden_tag2{},
          static_cast<std::integral_constant<
              meta::category_e,
              meta::category_e::ref_mov>*>(nullptr),
          tuple::get_inner(VEG_FWD(tup))){})

  VEG_TEMPLATE_EXPLICIT(
      !__VEG_ALL_OF(__VEG_CONCEPT(meta::convertible_to<Us&, Ts>)),
      (typename... Us),
      requires __VEG_ALL_OF(
          (__VEG_CONCEPT(meta::constructible<Ts, Us&>) &&
           !__VEG_CONCEPT(meta::constructible<Ts, Us const&>))),
      HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base,
      ((tup, veg::tuple<Us...>&)),
      noexcept(meta::all_of(
          {__VEG_CONCEPT(meta::nothrow_constructible<Ts, Us&>)...}))
      : m_impl(
          hidden_tag2{},
          static_cast<std::integral_constant<
              meta::category_e,
              meta::category_e::ref_mut>*>(nullptr),
          tuple::get_inner(VEG_FWD(tup))){})

  VEG_TEMPLATE_EXPLICIT(
      !__VEG_ALL_OF(__VEG_CONCEPT(meta::convertible_to<Us const&, Ts>)),
      (typename... Us),
      requires __VEG_ALL_OF(__VEG_CONCEPT(meta::constructible<Ts, Us const&>)),
      HEDLEY_ALWAYS_INLINE constexpr tuple_ctor_base,
      ((tup, veg::tuple<Us...> const&)),
      noexcept(meta::all_of(
          {__VEG_CONCEPT(meta::nothrow_constructible<Ts, Us const&>)...}))
      : m_impl(
          hidden_tag2{},
          static_cast<
              std::integral_constant<meta::category_e, meta::category_e::ref>*>(
              nullptr),
          tuple::get_inner(VEG_FWD(tup))){})

  __VEG_DEFAULTS(tuple_ctor_base);

private:
  internal::tuple::tuple_impl<meta::make_index_sequence<sizeof...(Ts)>, Ts...>
      m_impl;

  template <typename T>
  friend auto constexpr internal::tuple::get_inner(T&& tup) noexcept
      -> decltype((VEG_FWD(tup).m_impl))&&;
};

} // namespace tuple
} // namespace internal

template <typename... Ts>
struct tuple
    : internal::tuple::tuple_ctor_base<
          meta::all_of({__VEG_CONCEPT(meta::move_constructible<Ts>)...}),
          Ts...> {
  using base = internal::tuple::tuple_ctor_base<
      meta::all_of({__VEG_CONCEPT(meta::move_constructible<Ts>)...}),
      Ts...>;
  using base::base;
  using base::operator=;

  __VEG_DEFAULTS(tuple);

  template <typename... Us>
  tuple(veg::tuple<Us...> const&& tup) = delete;

  template <i64 I>
  void operator[](fix<I> /*arg*/) const&& = delete;

  VEG_TEMPLATE(
      (i64 I),
      requires(
          I < sizeof...(Ts) && (I >= 0) &&
          __VEG_CONCEPT(
              meta::move_constructible<typename internal::tuple::pack_ith_elem<
                  usize(I)>::template type<Ts...>>)),
      HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto
      operator[],
      (/*arg*/, fix<I>)) &&
      __VEG_DEDUCE_RET(
          internal::tuple::adl_get<I>(static_cast<tuple&&>(*this)));

  VEG_TEMPLATE(
      (i64 I),
      requires(I < sizeof...(Ts) && (I >= 0)),
      HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto
      operator[],
      (/*arg*/, fix<I>)) &
      __VEG_DEDUCE_RET(internal::tuple::adl_get<I>(*this));

  VEG_TEMPLATE(
      (i64 I),
      requires(I < sizeof...(Ts) && (I >= 0)),
      HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto
      operator[],
      (/*arg*/, fix<I>))
  const& __VEG_DEDUCE_RET(internal::tuple::adl_get<I>(*this));

  template <typename... Us>
  void operator=(internal::tuple::adl::tuple_base<Us...>&&) const& = delete;
  template <typename... Us>
  void
  operator=(internal::tuple::adl::tuple_base<Us...> const&) const& = delete;
  template <typename... Us>
  void operator=(internal::tuple::adl::tuple_base<Us...>&&) & = delete;
  template <typename... Us>
  void operator=(internal::tuple::adl::tuple_base<Us...> const&) & = delete;
  template <typename... Us>
  void operator=(internal::tuple::adl::tuple_base<Us...>&&) && = delete;
  template <typename... Us>
  void operator=(internal::tuple::adl::tuple_base<Us...> const&) && = delete;

  VEG_TEMPLATE(
      (typename... Us),
      requires __VEG_ALL_OF(
          !__VEG_CONCEPT(meta::reference<Ts>) &&
          __VEG_CONCEPT(meta::assignable<Ts&, Us const&>)),
      HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto
      operator=,
      (rhs, veg::tuple<Us...> const&)) &

      noexcept(meta::all_of(
          {__VEG_CONCEPT(meta::nothrow_assignable<Ts&, Us const&>)...}))
          -> tuple& {

    internal::tuple::binop_apply<
        internal::tuple::assign_ftor,
        meta::all_of(
            {__VEG_CONCEPT(meta::nothrow_assignable<Ts&, Us const&>)...})>(

        static_cast<tuple<Ts&...>*>(nullptr),
        static_cast<tuple<Us const&...>*>(nullptr),
        internal::tuple::get_inner(*this),
        internal::tuple::get_inner(rhs)

    );

    return *this;
  }

  VEG_TEMPLATE(
      (typename... Us),
      requires __VEG_ALL_OF(
          !__VEG_CONCEPT(meta::reference<Ts>) &&
          __VEG_CONCEPT(meta::assignable<Ts&, Us&&>)),
      HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto
      operator=,
      (rhs, veg::tuple<Us...>&&)) &

      noexcept(meta::all_of(
          {__VEG_CONCEPT(meta::nothrow_assignable<Ts&, Us&&>)...})) -> tuple& {

    internal::tuple::binop_apply<
        internal::tuple::assign_ftor,
        meta::all_of({__VEG_CONCEPT(meta::nothrow_assignable<Ts&, Us&&>)...})>(

        static_cast<tuple<Ts&...>*>(nullptr),
        static_cast<tuple<Us&&...>*>(nullptr),
        internal::tuple::get_inner(*this),
        internal::tuple::get_inner(rhs)

    );

    return *this;
  }

  VEG_TEMPLATE(
      (typename... Us),
      requires __VEG_ALL_OF(__VEG_CONCEPT(meta::assignable<Ts&&, Us const&>)),
      HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto
      operator=,
      (rhs, veg::tuple<Us...> const&)) &&

      noexcept(meta::all_of(
          {__VEG_CONCEPT(meta::nothrow_assignable<Ts&&, Us const&>)...}))
          -> tuple&& {

    internal::tuple::binop_apply<
        internal::tuple::assign_ftor,
        meta::all_of(
            {__VEG_CONCEPT(meta::nothrow_assignable<Ts&&, Us const&>)...})>(

        static_cast<tuple<Ts&&...>*>(nullptr),
        static_cast<tuple<Us const&...>*>(nullptr),
        internal::tuple::get_inner(*this),
        internal::tuple::get_inner(rhs)

    );

    return static_cast<tuple&&>(*this);
  }

  VEG_TEMPLATE(
      (typename... Us),
      requires __VEG_ALL_OF(__VEG_CONCEPT(meta::assignable<Ts&&, Us&&>)),
      HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto
      operator=,
      (rhs, veg::tuple<Us...>&&)) &&

      noexcept(meta::all_of(
          {__VEG_CONCEPT(meta::nothrow_assignable<Ts&&, Us&&>)...}))
          -> tuple&& {

    internal::tuple::binop_apply<
        internal::tuple::assign_ftor,
        meta::all_of({__VEG_CONCEPT(meta::nothrow_assignable<Ts&&, Us&&>)...})>(

        static_cast<tuple<Ts&&...>*>(nullptr),
        static_cast<tuple<Us&&...>*>(nullptr),
        internal::tuple::get_inner(*this),
        internal::tuple::get_inner(rhs)

    );

    return static_cast<tuple&&>(*this);
  }

  VEG_TEMPLATE(
      (typename... Us),
      requires __VEG_ALL_OF(
          __VEG_CONCEPT(meta::assignable<Ts const&, Us const&>)),
      HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto
      operator=,
      (rhs, veg::tuple<Us...> const&))
  const&

      noexcept(meta::all_of({__VEG_CONCEPT(
                   meta::nothrow_assignable<Ts const&, Us const&>)...}))
          ->tuple const& {

    internal::tuple::binop_apply<
        internal::tuple::assign_ftor,
        meta::all_of({__VEG_CONCEPT(
            meta::nothrow_assignable<Ts const&, Us const&>)...})>(

        static_cast<tuple<Ts const&...>*>(nullptr),
        static_cast<tuple<Us const&...>*>(nullptr),
        internal::tuple::get_inner(*this),
        internal::tuple::get_inner(rhs)

    );

    return *this;
  }

  VEG_TEMPLATE(
      (typename... Us),
      requires __VEG_ALL_OF(__VEG_CONCEPT(meta::assignable<Ts const&, Us&&>)),
      HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto
      operator=,
      (rhs, veg::tuple<Us...>&&))
  const&

      noexcept(meta::all_of({__VEG_CONCEPT(
                   meta::nothrow_assignable<Ts const&, Us&&>)...}))
          ->tuple const& {

    internal::tuple::binop_apply<
        internal::tuple::assign_ftor,
        meta::all_of(
            {__VEG_CONCEPT(meta::nothrow_assignable<Ts const&, Us&&>)...})>(

        static_cast<tuple<Ts&...>*>(nullptr),
        static_cast<tuple<Us&&...>*>(nullptr),
        internal::tuple::get_inner(*this),
        internal::tuple::get_inner(rhs)

    );

    return *this;
  }

  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto as_ref() & noexcept
      -> tuple<Ts&...> {
    return tuple::as_ref_impl<Ts&...>(
        *this, meta::make_index_sequence<sizeof...(Ts)>{});
  }
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto as_ref() const& noexcept
      -> tuple<Ts const&...> {
    return tuple::as_ref_impl<Ts const&...>(
        *this, meta::make_index_sequence<sizeof...(Ts)>{});
  }
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto as_ref() && noexcept
      -> tuple<Ts&&...> {
    return tuple::as_ref_impl<Ts&&...>(
        static_cast<tuple&&>(*this),
        meta::make_index_sequence<sizeof...(Ts)>{});
  }

private:
  template <typename... Us, usize... Is>
  static constexpr auto HEDLEY_ALWAYS_INLINE as_ref_impl(
      tuple<Ts...> const& self, meta::index_sequence<Is...> /*seq*/) noexcept
      -> veg::tuple<Us...> {
    return {
        elems,

        const_cast<Us> /* NOLINT */ (
            static_cast<internal::tuple::tuple_leaf<Is, Ts> const&>(
                internal::tuple::get_inner(self))
                .inner)

            ...};
  }
};

__VEG_CPP17(template <typename... Ts> tuple(Ts...) -> tuple<Ts...>;)

} // namespace veg

#endif /* end of include guard __VEG_TUPLE_GENERIC_HPP_DUSBI7AJS */
