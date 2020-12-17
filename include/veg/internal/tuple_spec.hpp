#define VEG_TUPLE_SPECIALIZATION_COUNT 9
#include "veg/internal/type_traits.hpp"
#include "veg/internal/meta_int.hpp"
namespace veg {
// clang-format off
using usize = decltype(sizeof(0));
namespace internal {
namespace tuple {
template <typename T> auto get() = delete;
template <usize I, typename T> auto adl_get(T&& arg)
noexcept(noexcept(get<I>(VEG_FWD(arg))))
-> decltype(get<I>(VEG_FWD(arg))) { return get<I>(VEG_FWD(arg)); }
template <typename Ts> struct element;
template <decltype(sizeof(int)) I> struct get_impl;

template <> struct get_impl<0> {
template <typename T> static constexpr auto apply(T&& tup) noexcept-> auto&& {
return static_cast<meta::conditional_t<
  !std::is_rvalue_reference<decltype(tup._0)>::value || !std::is_rvalue_reference<T&&>::value,
decltype((VEG_FWD(tup)._0)), decltype(tup._0)&&>>(tup._0); }
template <typename T0, typename... Ts>using type = T0; };
template <> struct get_impl<1> {
template <typename T> static constexpr auto apply(T&& tup) noexcept-> auto&& {
return static_cast<meta::conditional_t<
  !std::is_rvalue_reference<decltype(tup._1)>::value || !std::is_rvalue_reference<T&&>::value,
decltype((VEG_FWD(tup)._1)), decltype(tup._1)&&>>(tup._1); }
template <typename T0, typename T1, typename... Ts>using type = T1; };
template <> struct get_impl<2> {
template <typename T> static constexpr auto apply(T&& tup) noexcept-> auto&& {
return static_cast<meta::conditional_t<
  !std::is_rvalue_reference<decltype(tup._2)>::value || !std::is_rvalue_reference<T&&>::value,
decltype((VEG_FWD(tup)._2)), decltype(tup._2)&&>>(tup._2); }
template <typename T0, typename T1, typename T2, typename... Ts>using type = T2; };
template <> struct get_impl<3> {
template <typename T> static constexpr auto apply(T&& tup) noexcept-> auto&& {
return static_cast<meta::conditional_t<
  !std::is_rvalue_reference<decltype(tup._3)>::value || !std::is_rvalue_reference<T&&>::value,
decltype((VEG_FWD(tup)._3)), decltype(tup._3)&&>>(tup._3); }
template <typename T0, typename T1, typename T2, typename T3, typename... Ts>using type = T3; };
template <> struct get_impl<4> {
template <typename T> static constexpr auto apply(T&& tup) noexcept-> auto&& {
return static_cast<meta::conditional_t<
  !std::is_rvalue_reference<decltype(tup._4)>::value || !std::is_rvalue_reference<T&&>::value,
decltype((VEG_FWD(tup)._4)), decltype(tup._4)&&>>(tup._4); }
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename... Ts>using type = T4; };
template <> struct get_impl<5> {
template <typename T> static constexpr auto apply(T&& tup) noexcept-> auto&& {
return static_cast<meta::conditional_t<
  !std::is_rvalue_reference<decltype(tup._5)>::value || !std::is_rvalue_reference<T&&>::value,
decltype((VEG_FWD(tup)._5)), decltype(tup._5)&&>>(tup._5); }
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename... Ts>using type = T5; };
template <> struct get_impl<6> {
template <typename T> static constexpr auto apply(T&& tup) noexcept-> auto&& {
return static_cast<meta::conditional_t<
  !std::is_rvalue_reference<decltype(tup._6)>::value || !std::is_rvalue_reference<T&&>::value,
decltype((VEG_FWD(tup)._6)), decltype(tup._6)&&>>(tup._6); }
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename... Ts>using type = T6; };
template <> struct get_impl<7> {
template <typename T> static constexpr auto apply(T&& tup) noexcept-> auto&& {
return static_cast<meta::conditional_t<
  !std::is_rvalue_reference<decltype(tup._7)>::value || !std::is_rvalue_reference<T&&>::value,
decltype((VEG_FWD(tup)._7)), decltype(tup._7)&&>>(tup._7); }
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename... Ts>using type = T7; };

} // namespace tuple
} // namespace internal

template <typename... Ts> struct tuple;
template <> struct tuple<> {
};

template <typename T0> struct tuple<T0> {T0 _0;
template <usize I, typename T> friend constexpr auto get(T&& arg) noexcept
-> VEG_REQUIRES_RET2((I < 1) && meta::is_same_v<meta::remove_cvref_t<T>, tuple>,
decltype(internal::tuple::get_impl<I>::apply(VEG_FWD(arg))))
{ return internal::tuple::get_impl<I>::apply(VEG_FWD(arg)); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) & noexcept-> VEG_REQUIRES_RET((I < 1) && (I >= 0),
decltype(internal::tuple::adl_get<I>(*this))){ return internal::tuple::adl_get<I>(*this); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) const& noexcept-> VEG_REQUIRES_RET((I < 1) && (I >= 0),
decltype(internal::tuple::adl_get<I>(*this))){ return internal::tuple::adl_get<I>(*this); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) && noexcept-> VEG_REQUIRES_RET((I < 1) && (I >= 0),
decltype(internal::tuple::adl_get<I>(static_cast<tuple&&>(*this)))){ return internal::tuple::adl_get<I>(static_cast<tuple&&>(*this)); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) const&& noexcept-> VEG_REQUIRES_RET((I < 1) && (I >= 0),
decltype(internal::tuple::adl_get<I>(static_cast<tuple const&&>(*this)))){ return internal::tuple::adl_get<I>(static_cast<tuple const&&>(*this)); }
};

template <typename T0, typename T1> struct tuple<T0, T1> {T0 _0; T1 _1;
template <usize I, typename T> friend constexpr auto get(T&& arg) noexcept
-> VEG_REQUIRES_RET2((I < 2) && meta::is_same_v<meta::remove_cvref_t<T>, tuple>,
decltype(internal::tuple::get_impl<I>::apply(VEG_FWD(arg))))
{ return internal::tuple::get_impl<I>::apply(VEG_FWD(arg)); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) & noexcept-> VEG_REQUIRES_RET((I < 2) && (I >= 0),
decltype(internal::tuple::adl_get<I>(*this))){ return internal::tuple::adl_get<I>(*this); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) const& noexcept-> VEG_REQUIRES_RET((I < 2) && (I >= 0),
decltype(internal::tuple::adl_get<I>(*this))){ return internal::tuple::adl_get<I>(*this); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) && noexcept-> VEG_REQUIRES_RET((I < 2) && (I >= 0),
decltype(internal::tuple::adl_get<I>(static_cast<tuple&&>(*this)))){ return internal::tuple::adl_get<I>(static_cast<tuple&&>(*this)); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) const&& noexcept-> VEG_REQUIRES_RET((I < 2) && (I >= 0),
decltype(internal::tuple::adl_get<I>(static_cast<tuple const&&>(*this)))){ return internal::tuple::adl_get<I>(static_cast<tuple const&&>(*this)); }
};

template <typename T0, typename T1, typename T2> struct tuple<T0, T1, T2> {T0 _0; T1 _1; T2 _2;
template <usize I, typename T> friend constexpr auto get(T&& arg) noexcept
-> VEG_REQUIRES_RET2((I < 3) && meta::is_same_v<meta::remove_cvref_t<T>, tuple>,
decltype(internal::tuple::get_impl<I>::apply(VEG_FWD(arg))))
{ return internal::tuple::get_impl<I>::apply(VEG_FWD(arg)); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) & noexcept-> VEG_REQUIRES_RET((I < 3) && (I >= 0),
decltype(internal::tuple::adl_get<I>(*this))){ return internal::tuple::adl_get<I>(*this); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) const& noexcept-> VEG_REQUIRES_RET((I < 3) && (I >= 0),
decltype(internal::tuple::adl_get<I>(*this))){ return internal::tuple::adl_get<I>(*this); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) && noexcept-> VEG_REQUIRES_RET((I < 3) && (I >= 0),
decltype(internal::tuple::adl_get<I>(static_cast<tuple&&>(*this)))){ return internal::tuple::adl_get<I>(static_cast<tuple&&>(*this)); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) const&& noexcept-> VEG_REQUIRES_RET((I < 3) && (I >= 0),
decltype(internal::tuple::adl_get<I>(static_cast<tuple const&&>(*this)))){ return internal::tuple::adl_get<I>(static_cast<tuple const&&>(*this)); }
};

template <typename T0, typename T1, typename T2, typename T3> struct tuple<T0, T1, T2, T3> {T0 _0; T1 _1; T2 _2; T3 _3;
template <usize I, typename T> friend constexpr auto get(T&& arg) noexcept
-> VEG_REQUIRES_RET2((I < 4) && meta::is_same_v<meta::remove_cvref_t<T>, tuple>,
decltype(internal::tuple::get_impl<I>::apply(VEG_FWD(arg))))
{ return internal::tuple::get_impl<I>::apply(VEG_FWD(arg)); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) & noexcept-> VEG_REQUIRES_RET((I < 4) && (I >= 0),
decltype(internal::tuple::adl_get<I>(*this))){ return internal::tuple::adl_get<I>(*this); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) const& noexcept-> VEG_REQUIRES_RET((I < 4) && (I >= 0),
decltype(internal::tuple::adl_get<I>(*this))){ return internal::tuple::adl_get<I>(*this); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) && noexcept-> VEG_REQUIRES_RET((I < 4) && (I >= 0),
decltype(internal::tuple::adl_get<I>(static_cast<tuple&&>(*this)))){ return internal::tuple::adl_get<I>(static_cast<tuple&&>(*this)); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) const&& noexcept-> VEG_REQUIRES_RET((I < 4) && (I >= 0),
decltype(internal::tuple::adl_get<I>(static_cast<tuple const&&>(*this)))){ return internal::tuple::adl_get<I>(static_cast<tuple const&&>(*this)); }
};

template <typename T0, typename T1, typename T2, typename T3, typename T4> struct tuple<T0, T1, T2, T3, T4> {T0 _0; T1 _1; T2 _2; T3 _3; T4 _4;
template <usize I, typename T> friend constexpr auto get(T&& arg) noexcept
-> VEG_REQUIRES_RET2((I < 5) && meta::is_same_v<meta::remove_cvref_t<T>, tuple>,
decltype(internal::tuple::get_impl<I>::apply(VEG_FWD(arg))))
{ return internal::tuple::get_impl<I>::apply(VEG_FWD(arg)); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) & noexcept-> VEG_REQUIRES_RET((I < 5) && (I >= 0),
decltype(internal::tuple::adl_get<I>(*this))){ return internal::tuple::adl_get<I>(*this); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) const& noexcept-> VEG_REQUIRES_RET((I < 5) && (I >= 0),
decltype(internal::tuple::adl_get<I>(*this))){ return internal::tuple::adl_get<I>(*this); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) && noexcept-> VEG_REQUIRES_RET((I < 5) && (I >= 0),
decltype(internal::tuple::adl_get<I>(static_cast<tuple&&>(*this)))){ return internal::tuple::adl_get<I>(static_cast<tuple&&>(*this)); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) const&& noexcept-> VEG_REQUIRES_RET((I < 5) && (I >= 0),
decltype(internal::tuple::adl_get<I>(static_cast<tuple const&&>(*this)))){ return internal::tuple::adl_get<I>(static_cast<tuple const&&>(*this)); }
};

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5> struct tuple<T0, T1, T2, T3, T4, T5> {T0 _0; T1 _1; T2 _2; T3 _3; T4 _4; T5 _5;
template <usize I, typename T> friend constexpr auto get(T&& arg) noexcept
-> VEG_REQUIRES_RET2((I < 6) && meta::is_same_v<meta::remove_cvref_t<T>, tuple>,
decltype(internal::tuple::get_impl<I>::apply(VEG_FWD(arg))))
{ return internal::tuple::get_impl<I>::apply(VEG_FWD(arg)); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) & noexcept-> VEG_REQUIRES_RET((I < 6) && (I >= 0),
decltype(internal::tuple::adl_get<I>(*this))){ return internal::tuple::adl_get<I>(*this); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) const& noexcept-> VEG_REQUIRES_RET((I < 6) && (I >= 0),
decltype(internal::tuple::adl_get<I>(*this))){ return internal::tuple::adl_get<I>(*this); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) && noexcept-> VEG_REQUIRES_RET((I < 6) && (I >= 0),
decltype(internal::tuple::adl_get<I>(static_cast<tuple&&>(*this)))){ return internal::tuple::adl_get<I>(static_cast<tuple&&>(*this)); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) const&& noexcept-> VEG_REQUIRES_RET((I < 6) && (I >= 0),
decltype(internal::tuple::adl_get<I>(static_cast<tuple const&&>(*this)))){ return internal::tuple::adl_get<I>(static_cast<tuple const&&>(*this)); }
};

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> struct tuple<T0, T1, T2, T3, T4, T5, T6> {T0 _0; T1 _1; T2 _2; T3 _3; T4 _4; T5 _5; T6 _6;
template <usize I, typename T> friend constexpr auto get(T&& arg) noexcept
-> VEG_REQUIRES_RET2((I < 7) && meta::is_same_v<meta::remove_cvref_t<T>, tuple>,
decltype(internal::tuple::get_impl<I>::apply(VEG_FWD(arg))))
{ return internal::tuple::get_impl<I>::apply(VEG_FWD(arg)); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) & noexcept-> VEG_REQUIRES_RET((I < 7) && (I >= 0),
decltype(internal::tuple::adl_get<I>(*this))){ return internal::tuple::adl_get<I>(*this); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) const& noexcept-> VEG_REQUIRES_RET((I < 7) && (I >= 0),
decltype(internal::tuple::adl_get<I>(*this))){ return internal::tuple::adl_get<I>(*this); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) && noexcept-> VEG_REQUIRES_RET((I < 7) && (I >= 0),
decltype(internal::tuple::adl_get<I>(static_cast<tuple&&>(*this)))){ return internal::tuple::adl_get<I>(static_cast<tuple&&>(*this)); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) const&& noexcept-> VEG_REQUIRES_RET((I < 7) && (I >= 0),
decltype(internal::tuple::adl_get<I>(static_cast<tuple const&&>(*this)))){ return internal::tuple::adl_get<I>(static_cast<tuple const&&>(*this)); }
};

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> struct tuple<T0, T1, T2, T3, T4, T5, T6, T7> {T0 _0; T1 _1; T2 _2; T3 _3; T4 _4; T5 _5; T6 _6; T7 _7;
template <usize I, typename T> friend constexpr auto get(T&& arg) noexcept
-> VEG_REQUIRES_RET2((I < 8) && meta::is_same_v<meta::remove_cvref_t<T>, tuple>,
decltype(internal::tuple::get_impl<I>::apply(VEG_FWD(arg))))
{ return internal::tuple::get_impl<I>::apply(VEG_FWD(arg)); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) & noexcept-> VEG_REQUIRES_RET((I < 8) && (I >= 0),
decltype(internal::tuple::adl_get<I>(*this))){ return internal::tuple::adl_get<I>(*this); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) const& noexcept-> VEG_REQUIRES_RET((I < 8) && (I >= 0),
decltype(internal::tuple::adl_get<I>(*this))){ return internal::tuple::adl_get<I>(*this); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) && noexcept-> VEG_REQUIRES_RET((I < 8) && (I >= 0),
decltype(internal::tuple::adl_get<I>(static_cast<tuple&&>(*this)))){ return internal::tuple::adl_get<I>(static_cast<tuple&&>(*this)); }
template <i64 I> constexpr auto operator[](fix<I> /*arg*/) const&& noexcept-> VEG_REQUIRES_RET((I < 8) && (I >= 0),
decltype(internal::tuple::adl_get<I>(static_cast<tuple const&&>(*this)))){ return internal::tuple::adl_get<I>(static_cast<tuple const&&>(*this)); }
};

// clang-format on
} // namespace veg
