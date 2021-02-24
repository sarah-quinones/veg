#ifndef __VEG_FN_REF_HPP_IDVWA3EYS
#define __VEG_FN_REF_HPP_IDVWA3EYS

#include "veg/assert.hpp"

namespace veg {

template <typename T>
struct mini_fn_ref;
template <typename T>
struct fn_ref;

namespace internal {
namespace fnref {

struct dummy {};

union sstate_t {
  void* ptr;
  void (*fn)();
};
union state_t {
  void* ptr;
  void (*fn)();
  void (dummy::*mem_fn)();
};

template <typename T>
struct is_fn_ptr : std::false_type {};
template <typename Ret, typename... Args>
struct is_fn_ptr<Ret (*)(Args...)> : std::true_type {};

template <typename T>
using unary_plus = decltype(+__VEG_DECLVAL(T));

template <typename T>
struct is_convertible_to_fn_ptr_with_unary_plus
    : is_fn_ptr<meta::detected_t<unary_plus, T>> {};

template <typename Ret>
struct discard_void {
  // non void case
  template <typename Fn, typename... Args>
  static auto apply(Fn&& fn, Args&&... args) -> Ret {
    return invoke(VEG_FWD(fn), VEG_FWD(args)...);
  }
};
template <>
struct discard_void<void> {
  template <typename Fn, typename... Args>
  static auto apply(Fn&& fn, Args&&... args) -> void {
    void(invoke(VEG_FWD(fn), VEG_FWD(args)...));
  }
};

enum struct fn_kind_e {
  fn_ptr,
  mem_fn_ptr,
  fn_obj,
};

template <typename T>
struct fn_kind : std::integral_constant<
                     fn_kind_e,
                     is_convertible_to_fn_ptr_with_unary_plus<T>::value
                         ? fn_kind_e::fn_ptr
                         : fn_kind_e::fn_obj> {};

template <typename Ret, typename... Args>
struct fn_kind<Ret (*)(Args...)>
    : std::integral_constant<fn_kind_e, fn_kind_e::fn_ptr> {};

// member functions were a mistake
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...)>
    : std::integral_constant<fn_kind_e, fn_kind_e::mem_fn_ptr> {};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) const>
    : std::integral_constant<fn_kind_e, fn_kind_e::mem_fn_ptr> {};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...)&>
    : std::integral_constant<fn_kind_e, fn_kind_e::mem_fn_ptr> {};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) const&>
    : std::integral_constant<fn_kind_e, fn_kind_e::mem_fn_ptr> {};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) &&>
    : std::integral_constant<fn_kind_e, fn_kind_e::mem_fn_ptr> {};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) const&&>
    : std::integral_constant<fn_kind_e, fn_kind_e::mem_fn_ptr> {};

#if __cplusplus >= 201703L

template <typename Ret, typename... Args>
struct fn_kind<Ret (*)(Args...) noexcept>
    : std::integral_constant<fn_kind_e, fn_kind_e::fn_ptr> {};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) noexcept>
    : std::integral_constant<fn_kind_e, fn_kind_e::mem_fn_ptr> {};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) const noexcept>
    : std::integral_constant<fn_kind_e, fn_kind_e::mem_fn_ptr> {};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...)& noexcept>
    : std::integral_constant<fn_kind_e, fn_kind_e::mem_fn_ptr> {};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) const& noexcept>
    : std::integral_constant<fn_kind_e, fn_kind_e::mem_fn_ptr> {};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...)&& noexcept>
    : std::integral_constant<fn_kind_e, fn_kind_e::mem_fn_ptr> {};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) const&& noexcept>
    : std::integral_constant<fn_kind_e, fn_kind_e::mem_fn_ptr> {};

#endif

template <fn_kind_e Kind>
struct fn_ref_impl;

template <>
struct fn_ref_impl<fn_kind_e::fn_obj> {
  template <typename State, typename Fn>
  static auto address(Fn& arg) -> State {
    using char_ref = char&;
    using vptr = void*;
    using ptr = Fn*;
    return {ptr(vptr(&char_ref(arg)))};
  }
  template <typename State>
  static auto is_null(State /*arg*/) -> bool {
    return false;
  }

  template <typename State, typename Fn, typename Ret, typename... Args>
  static auto call(State state, Args... args) -> Ret {
    return discard_void<Ret>::apply(
        VEG_FWD(*static_cast<meta::remove_cvref_t<Fn>*>(state.ptr)),
        VEG_FWD(args)...);
  }
};
template <>
struct fn_ref_impl<fn_kind_e::fn_ptr> {
  template <typename State, typename Fn>
  static auto address(Fn& arg) -> State {
    State rv;
    rv.fn = reinterpret_cast<void (*)()>(+arg);
    return rv;
  }
  template <typename State>
  static auto is_null(State arg) -> bool {
    return arg.fn == nullptr;
  }

  template <typename State, typename Fn, typename Ret, typename... Args>
  static auto call(State state, Args... args) -> Ret {
    return discard_void<Ret>::apply(
        reinterpret_cast<decltype(+__VEG_DECLVAL(Fn&))>(state.fn),
        VEG_FWD(args)...);
  }
};

template <>
struct fn_ref_impl<fn_kind_e::mem_fn_ptr> {
  template <typename State, typename Fn>
  static auto address(Fn& arg) -> state_t {
    state_t rv{};
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

    rv.mem_fn = reinterpret_cast<void (dummy::*)()>(arg);

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

    return rv;
  }
  template <typename State>
  static auto is_null(State arg) -> bool {
    return arg.mem_fn == nullptr;
  }

  template <typename State, typename Fn, typename Ret, typename... Args>
  static auto call(State state, Args... args) -> Ret {
    return discard_void<Ret>::apply(
        reinterpret_cast<decltype(__VEG_DECLVAL(Fn&))>(state.mem_fn),
        VEG_FWD(args)...);
  }
};

template <
    typename State,
    template <typename Fn, typename... Args>
    class Concept,
    typename Ret,
    typename... Args>
struct function_ref_impl {

  constexpr function_ref_impl() = default;
  // COMPAT: check if function_ref_impl is a base of Fn
  VEG_TEMPLATE(
      (typename Fn),
      requires(
          meta::void_<Ret>::value ||
          meta::convertible_to<Ret, meta::detected_t<Concept, Fn, Args...>>::
              value),
      function_ref_impl, /* NOLINT(hicpp-explicit-conversions,
                           bugprone-forwarding-reference-overload) */
      (fn, Fn&&))
  noexcept
      : m_state(fnref::fn_ref_impl<fnref::fn_kind<meta::decay_t<Fn>>::value>::
                    template address<State>(fn)),
        m_call(fnref::fn_ref_impl<fnref::fn_kind<meta::decay_t<Fn>>::value>::
                   template call<State, Fn, Ret, Args...>) {
    VEG_ASSERT(
        !fnref::fn_ref_impl<fnref::fn_kind<meta::decay_t<Fn>>::value>::is_null(
            m_state));
  }

  auto operator()(Args... args) const -> Ret {
    VEG_DEBUG_ASSERT(m_call != nullptr);
    return this->m_call(this->m_state, VEG_FWD(args)...);
  }

  template <fnref::fn_kind_e>
  friend struct fnref::fn_ref_impl;

  State m_state = {nullptr};
  auto (*m_call)(State, Args...) -> Ret = nullptr;
};

} // namespace fnref
} // namespace internal

template <typename Ret, typename... Args>
struct mini_fn_ref<Ret(Args...)> : private internal::fnref::function_ref_impl<
                                       internal::fnref::sstate_t,
                                       meta::call_result_t,
                                       Ret,
                                       Args...> {
private:
  using base = internal::fnref::function_ref_impl<
      internal::fnref::sstate_t,
      meta::call_result_t,
      Ret,
      Args...>;
  explicit constexpr mini_fn_ref(internal::fnref::dummy /*unused*/) : base(){};
  template <typename U>
  friend struct meta::value_sentinel_for;

public:
  mini_fn_ref() = delete;
  using base ::base;
  using base::operator();
};

template <typename Ret, typename... Args>
struct fn_ref<Ret(Args...)> : private internal::fnref::function_ref_impl<
                                  internal::fnref::state_t,
                                  meta::invoke_result_t,
                                  Ret,
                                  Args...> {
private:
  using base = internal::fnref::function_ref_impl<
      internal::fnref::state_t,
      meta::invoke_result_t,
      Ret,
      Args...>;
  explicit constexpr fn_ref(internal::fnref::dummy /*unused*/) : base(){};
  template <typename U>
  friend struct meta::value_sentinel_for;

public:
  fn_ref() = delete;
  using base ::base;
  using base::operator();
};

template <typename Ret, typename... Args>
struct meta::value_sentinel_for<fn_ref<Ret(Args...)>>
    : std::integral_constant<i64, 1> {
  static constexpr auto invalid(i64 i) noexcept -> fn_ref<Ret(Args...)> {
    return VEG_ASSERT(i == 0),
           fn_ref<Ret(Args...)>{::veg::internal::fnref::dummy{}};
  }
  static constexpr auto id(fn_ref<Ret(Args...)> arg) -> i64 {
    return arg.m_call == nullptr ? 0 : -1;
  }
};
template <typename Ret, typename... Args>
struct meta::value_sentinel_for<mini_fn_ref<Ret(Args...)>> : std::true_type {
  static constexpr auto invalid(i64 i) noexcept -> mini_fn_ref<Ret(Args...)> {
    return VEG_ASSERT(i == 0),
           mini_fn_ref<Ret(Args...)>{::veg::internal::fnref::dummy{}};
  }
  static constexpr auto id(mini_fn_ref<Ret(Args...)> arg) -> i64 {
    return arg.m_call == nullptr ? 0 : -1;
  }
};

} // namespace veg

#endif /* end of include guard __VEG_FN_REF_HPP_IDVWA3EYS */
