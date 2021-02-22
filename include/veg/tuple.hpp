#ifndef VEG_TUPLE_HPP_B8PHUNWES
#define VEG_TUPLE_HPP_B8PHUNWES

#include "veg/internal/tuple_generic.hpp"
#include <utility> // std::tuple_{size,element}

/******************************************************************************/
#define VEG_IMPL_BIND(r, Tuple, Index, Identifier)                             \
  auto&& Identifier = ::veg::get<Index>(VEG_FWD(Tuple));

#define VEG_IMPL_BIND_ID_SEQ(CV_Auto, Identifiers, Tuple, SeqSize, TupleId)    \
  CV_Auto TupleId = Tuple;                                                     \
  static_assert(                                                               \
      ::std::tuple_size<                                                       \
          typename ::std::remove_const<typename ::std::remove_reference<       \
              decltype(TupleId)>::type>::type>::value == SeqSize,              \
      "Wrong number of identifiers.");                                         \
  VEG_PP_SEQ_FOR_EACH_I(VEG_IMPL_BIND, TupleId, Identifiers)                   \
  static_assert(true, "")

// example: difference vs c++17 structure bindings
// auto get() -> tuple<A, B&, C&&>;
//
// auto [a, b, c] = get();
// VEG_BIND(auto, (x, y, z), get());
// decltype({a,b,c}) => {A,B&,C&&}     same as tuple_element<i, E>
// decltype({x,y,z}) => {A&&,B&,C&&}   always a reference, lvalue if initializer
//                                     expression or tuple_element<i, E> is an
//                                     lvalue, rvalue otherwise.
//
#define VEG_BIND(CV_Auto, Identifiers, Tuple)                                  \
  VEG_IMPL_BIND_ID_SEQ(                                                        \
      CV_Auto,                                                                 \
      VEG_PP_TUPLE_TO_SEQ(Identifiers),                                        \
      Tuple,                                                                   \
      VEG_PP_TUPLE_SIZE(Identifiers),                                          \
      VEG_PP_CAT(_dummy_tuple_variable_id_, __LINE__))
/******************************************************************************/

namespace veg {
namespace internal {
namespace get {

template <typename T>
void get() = delete;

struct member_get {
  template <usize I, typename T>
  using type = decltype(void(VEG_DECLVAL(T).template get<I>()));
  template <usize I, typename T>
  static constexpr auto apply(T&& arg)
      VEG_DEDUCE_RET(VEG_FWD(arg).template get<I>());
};
struct adl_get {
  template <usize I, typename T>
  using type = decltype(void(get<I>(VEG_DECLVAL(T))));

  template <usize I, typename T>
  static constexpr auto apply(T&& arg) VEG_DEDUCE_RET(get<I>(VEG_FWD(arg)));
};

template <usize I, typename T>
struct has_member_get : meta::is_detected_i<member_get::type, I, T&&>,
                        member_get {};
template <usize I, typename T>
struct has_adl_get : meta::is_detected_i<adl_get::type, I, T&&>, adl_get {};

template <usize I, typename T>
struct get_impl : meta::disjunction<has_member_get<I, T>, has_adl_get<I, T>> {};
} // namespace get
} // namespace internal

namespace fn {
template <i64 I>
struct get_fn {
  VEG_TEMPLATE(
      typename T,
      requires(internal::get::get_impl<static_cast<usize>(I), T>::value),
      constexpr auto
      operator(),
      (arg, T&&))
  const VEG_DEDUCE_RET(
      internal::get::get_impl<static_cast<usize>(I), T>::template apply<I>(
          VEG_FWD(arg)));
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
  VEG_TEMPLATE(
      typename... Ts,
      requires_all(meta::constructible<meta::decay_t<Ts>, Ts&&>::value),
      constexpr auto
      operator(),
      (... args, Ts&&))
  const noexcept(
      meta::all_of(
          {meta::nothrow_constructible<meta::decay_t<Ts>, Ts&&>::value...}))
      ->tuple<meta::decay_t<Ts>...> {
    return {VEG_FWD(args)...};
  }
};

struct tuple_ref_fn {
  VEG_TEMPLATE(
      typename... Ts,
      requires true,
      constexpr auto
      operator(),
      (... args, Ts&&))
  const noexcept->tuple<Ts&&...> { return {VEG_FWD(args)...}; }
};

struct tuple_fwd_fn {
  VEG_TEMPLATE(
      typename... Ts,
      requires_all(meta::constructible<Ts, Ts&&>::value),
      constexpr auto
      operator(),
      (... args, Ts&&))
  const noexcept(
      meta::all_of({meta::nothrow_constructible<Ts, Ts&&>::value...}))
      ->tuple<Ts...> {
    return {VEG_FWD(args)...};
  }
};
} // namespace fn
VEG_ODR_VAR(tuple, fn::tuple_fn);
VEG_ODR_VAR(tuple_ref, fn::tuple_ref_fn);
VEG_ODR_VAR(tuple_fwd, fn::tuple_fwd_fn);

} // namespace make
namespace meta {
template <typename... Ts>
struct trivially_relocatable<tuple<Ts...>>
    : bool_constant<all_of({trivially_relocatable<Ts>::value...})> {};

template <typename... Ts>
struct mostly_trivial<tuple<Ts...>>
    : bool_constant<all_of({mostly_trivial<Ts>::value...})> {};
} // namespace meta
} // namespace veg

namespace std {
template <typename... Ts>
struct tuple_size<veg::tuple<Ts...>>
    : integral_constant<veg::meta::usize, sizeof...(Ts)> {};

template <veg::meta::usize I, typename... Ts>
struct tuple_element<I, veg::tuple<Ts...>> {
  using type =
      typename veg::internal::tuple::pack_ith_elem<I>::template type<Ts...>;
};
} // namespace std

#endif /* end of include guard VEG_TUPLE_HPP_B8PHUNWES */
