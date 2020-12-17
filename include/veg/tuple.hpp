#ifndef VEG_TUPLE_HPP_B8PHUNWES
#define VEG_TUPLE_HPP_B8PHUNWES

#include "veg/internal/tuple_generic.hpp"

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
