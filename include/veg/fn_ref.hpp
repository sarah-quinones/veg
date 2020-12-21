#ifndef VEG_FN_REF_HPP_IDVWA3EYS
#define VEG_FN_REF_HPP_IDVWA3EYS

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
struct is_fn_ptr {
  static constexpr bool value = false;
};
template <typename Ret, typename... Args>
struct is_fn_ptr<Ret (*)(Args...)> {
  static constexpr bool value = true;
};

template <typename T, typename Enable = void>
struct is_convertible_to_fn_ptr_with_unary_plus {
  static constexpr bool value = false;
};
template <typename T>
struct is_convertible_to_fn_ptr_with_unary_plus<
    T,
    decltype(static_cast<void>(+VEG_DECLVAL(T)))> {
  static constexpr bool value =
      is_fn_ptr<meta::decay_t<decltype(+VEG_DECLVAL(T))>>::value;
};

template <typename Enable, typename Fn, typename To, typename... Args>
struct call_r_impl {
  static constexpr bool value = false;
};
template <typename To, typename Fn, typename... Args>
struct call_r_impl<
    meta::enable_if_t<
        meta::is_same<To, void>::value ||
        meta::is_convertible<
            decltype(VEG_DECLVAL(Fn)(VEG_DECLVAL(Args)...)),
            To>::value>,
    Fn,
    To,
    Args...> {
  static constexpr bool value = true;

  static auto apply(Fn&& fn, Args&&... args)
      -> decltype(VEG_FWD(fn)(VEG_FWD(args)...)) {
    return VEG_FWD(fn)(VEG_FWD(args)...);
  }
};

template <typename Enable, typename Fn, typename To, typename... Args>
struct invoke_r_impl : call_r_impl<Enable, Fn, To, Args...> {};

template <typename To, typename Mem_Fn, typename First, typename... Args>
struct invoke_r_impl<
    meta::enable_if_t<
        meta::is_same<To, void>::value ||
        meta::is_convertible<
            decltype((VEG_DECLVAL(First).*VEG_DECLVAL(Mem_Fn))(
                VEG_DECLVAL(Args)...)),
            To>::value>,
    Mem_Fn,
    To,
    First,
    Args...> {

  static constexpr bool value = true;

  static auto apply(Mem_Fn fn, First&& first, Args&&... args)
      -> decltype((VEG_FWD(first).*fn)(VEG_FWD(args)...)) {
    return (VEG_FWD(first).*fn)(VEG_FWD(args)...);
  }
};

template <typename Fn, typename To, typename... Args>
struct is_invoke_convertible : invoke_r_impl<void, Fn, To, Args...> {};
template <typename Fn, typename To, typename... Args>
struct is_call_convertible : call_r_impl<void, Fn, To, Args...> {};

template <typename Ret>
struct discard_void {
  // non void case
  template <typename Fn, typename... Args>
  static auto apply(Fn&& fn, Args&&... args) -> Ret {
    return is_invoke_convertible<Fn, Ret, Args...>::apply(
        VEG_FWD(fn), VEG_FWD(args)...);
  }
};
template <>
struct discard_void<void> {
  template <typename Fn, typename... Args>
  static auto apply(Fn&& fn, Args&&... args) -> void {
    is_invoke_convertible<Fn, void, Args...>::apply(
        VEG_FWD(fn), VEG_FWD(args)...);
  }
};

enum struct fn_kind_e {
  fn_ptr,
  mem_fn_ptr,
  fn_obj,
};

template <typename T>
struct fn_kind {
  static constexpr fn_kind_e value =
      is_convertible_to_fn_ptr_with_unary_plus<T>::value ? fn_kind_e::fn_ptr
                                                         : fn_kind_e::fn_obj;
};

template <typename Ret, typename... Args>
struct fn_kind<Ret (*)(Args...)> {
  static constexpr fn_kind_e value = fn_kind_e::fn_ptr;
};

// member functions were a mistake
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...)> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) const> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...)&> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) const&> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) &&> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) const&&> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};

#if __cplusplus >= 201703L

template <typename Ret, typename... Args>
struct fn_kind<Ret (*)(Args...) noexcept> {
  static constexpr fn_kind_e value = fn_kind_e::fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) noexcept> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) const noexcept> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...)& noexcept> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) const& noexcept> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...)&& noexcept> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) const&& noexcept> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};

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
        reinterpret_cast<decltype(+VEG_DECLVAL(Fn&))>(state.fn),
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
        reinterpret_cast<decltype(VEG_DECLVAL(Fn&))>(state.mem_fn),
        VEG_FWD(args)...);
  }
};

template <
    typename State,
    template <typename Ret, typename Fn, typename... Args>
    class Concept,
    typename Ret,
    typename... Args>
struct function_ref_impl {

  constexpr function_ref_impl() = default;
  // COMPAT: check if function_ref_impl is a base of Fn
  VEG_TEMPLATE(
      (typename Fn),
      requires(Concept<Fn, Ret, Args...>::value),
      function_ref_impl, /* NOLINT(hicpp-explicit-conversions,
                           bugprone-forwarding-reference-overload) */
      (fn, Fn&&))
  noexcept
      : m_state(fnref::fn_ref_impl<fnref::fn_kind<meta::decay_t<Fn>>::value>::
                    template address<State>(fn)),
        m_call(fnref::fn_ref_impl<fnref::fn_kind<meta::decay_t<Fn>>::value>::
                   template call<State, Fn, Ret, Args...>) {
    if (fnref::fn_ref_impl<fnref::fn_kind<meta::decay_t<Fn>>::value>::is_null(
            m_state)) {
      m_call = nullptr;
    }
  }

  auto operator()(Args... args) const -> Ret {
    VEG_ASSERT(m_call != nullptr);
    return this->m_call(this->m_state, VEG_FWD(args)...);
  }

  constexpr explicit operator bool() const { return m_call != nullptr; }

private:
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
                                       internal::fnref::is_call_convertible,
                                       Ret,
                                       Args...> {
private:
  using base = internal::fnref::function_ref_impl<
      internal::fnref::sstate_t,
      internal::fnref::is_call_convertible,
      Ret,
      Args...>;

public:
  using base ::base;
  using base::operator();
  using base::operator bool;
};

template <typename Ret, typename... Args>
struct fn_ref<Ret(Args...)> : private internal::fnref::function_ref_impl<
                                  internal::fnref::state_t,
                                  internal::fnref::is_invoke_convertible,
                                  Ret,
                                  Args...> {
private:
  using base = internal::fnref::function_ref_impl<
      internal::fnref::state_t,
      internal::fnref::is_invoke_convertible,
      Ret,
      Args...>;

public:
  using base ::base;
  using base::operator();
  using base::operator bool;
};

} // namespace veg

#endif /* end of include guard VEG_FN_REF_HPP_IDVWA3EYS */
