#ifndef __VEG_TUPLE_HPP_B8PHUNWES
#define __VEG_TUPLE_HPP_B8PHUNWES

#include "veg/internal/tuple_generic.hpp"
#include <utility> // std::tuple_{size,element}

/******************************************************************************/
#define __VEG_IMPL_BIND(r, Tuple, Index, Identifier)                           \
  auto&& Identifier = ::veg::get<Index>(VEG_FWD(Tuple));

#define __VEG_IMPL_BIND_ID_SEQ(CV_Auto, Identifiers, Tuple, SeqSize, TupleId)  \
  CV_Auto TupleId = Tuple;                                                     \
  static_assert(                                                               \
      ::std::tuple_size<                                                       \
          typename ::std::remove_const<typename ::std::remove_reference<       \
              decltype(TupleId)>::type>::type>::value == SeqSize,              \
      "Wrong number of identifiers.");                                         \
  __VEG_PP_SEQ_FOR_EACH_I(__VEG_IMPL_BIND, TupleId, Identifiers)               \
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
  __VEG_IMPL_BIND_ID_SEQ(                                                      \
      CV_Auto,                                                                 \
      __VEG_PP_TUPLE_TO_SEQ(Identifiers),                                      \
      Tuple,                                                                   \
      __VEG_PP_TUPLE_SIZE(Identifiers),                                        \
      __VEG_PP_CAT(_dummy_tuple_variable_id_, __LINE__))
/******************************************************************************/

namespace veg {

namespace make {
namespace fn {
struct tuple : elems_t {};

struct tuple_ref {
  VEG_TEMPLATE(
      typename... Ts,
      requires true,
      HEDLEY_ALWAYS_INLINE constexpr auto
      operator(),
      (... args, Ts&&))
  const noexcept->veg::tuple<Ts&&...> {
    return veg::tuple<Ts&&...>{VEG_FWD(args)...};
  }
};

struct tuple_fwd {
  VEG_TEMPLATE(
      typename... Ts,
      requires_all(meta::constructible<Ts, Ts&&>::value),
      constexpr auto
      operator(),
      (... args, Ts&&))
  const noexcept(
      meta::all_of({meta::nothrow_constructible<Ts, Ts&&>::value...}))
      ->veg::tuple<Ts...> {
    return veg::tuple<Ts...>{VEG_FWD(args)...};
  }
};
} // namespace fn
__VEG_ODR_VAR(tuple, fn::tuple);
__VEG_ODR_VAR(tuple_ref, fn::tuple_ref);
__VEG_ODR_VAR(tuple_fwd, fn::tuple_fwd);

struct elems_t {};
__VEG_ODR_VAR(elems, elems_t);

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

#endif /* end of include guard __VEG_TUPLE_HPP_B8PHUNWES */
