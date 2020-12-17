#define VEG_TUPLE_SPECIALIZATION_COUNT 6
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
template <typename T> static constexpr auto apply(T&& tup) noexcept-> auto&& { return (VEG_FWD(tup)._0); }
template <typename T0, typename... Ts>using type = T0; };
template <> struct get_impl<1> {
template <typename T> static constexpr auto apply(T&& tup) noexcept-> auto&& { return (VEG_FWD(tup)._1); }
template <typename T0, typename T1, typename... Ts>using type = T1; };
template <> struct get_impl<2> {
template <typename T> static constexpr auto apply(T&& tup) noexcept-> auto&& { return (VEG_FWD(tup)._2); }
template <typename T0, typename T1, typename T2, typename... Ts>using type = T2; };
template <> struct get_impl<3> {
template <typename T> static constexpr auto apply(T&& tup) noexcept-> auto&& { return (VEG_FWD(tup)._3); }
template <typename T0, typename T1, typename T2, typename T3, typename... Ts>using type = T3; };
template <> struct get_impl<4> {
template <typename T> static constexpr auto apply(T&& tup) noexcept-> auto&& { return (VEG_FWD(tup)._4); }
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename... Ts>using type = T4; };

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

// clang-format on
} // namespace veg
