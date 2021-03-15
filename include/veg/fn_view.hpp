#ifndef __VEG_FN_REF_HPP_IDVWA3EYS
#define __VEG_FN_REF_HPP_IDVWA3EYS

#include "veg/assert.hpp"

namespace veg {

template <typename T>
struct fn_view;
template <typename T>
struct once_fn_view;

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
  static auto apply(Fn&& fn, Args&&... args) noexcept(
      __VEG_CONCEPT(meta::nothrow_invocable<Fn&&, Args&&...>)) -> Ret {
    return invoke(VEG_FWD(fn), VEG_FWD(args)...);
  }
};
template <>
struct discard_void<void> {
  template <typename Fn, typename... Args>
  static auto apply(Fn&& fn, Args&&... args) noexcept(
      __VEG_CONCEPT(meta::nothrow_invocable<Fn&&, Args&&...>)) -> void {
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
struct fn_view_impl;

template <>
struct fn_view_impl<fn_kind_e::fn_obj> {
  template <typename State, typename Fn>
  static auto address(Fn& arg) noexcept -> State {
    using char_ref = char&;
    using vptr = void*;
    using ptr = Fn*;
    return {ptr(vptr(&char_ref(arg)))};
  }
  template <typename State>
  static auto is_null(State /*arg*/) noexcept -> bool {
    return false;
  }

  template <typename State, typename Fn, typename Ret, typename... Args>
  static auto call(State state, Args... args) noexcept(
      __VEG_CONCEPT(meta::nothrow_invocable<Fn&&, Args&&...>)) -> Ret {
    return discard_void<Ret>::apply(
        VEG_FWD(*static_cast<meta::remove_cvref_t<Fn>*>(state.ptr)),
        VEG_FWD(args)...);
  }
};
template <>
struct fn_view_impl<fn_kind_e::fn_ptr> {
  template <typename State, typename Fn>
  static auto address(Fn& arg) noexcept -> State {
    State rv;
    rv.fn = reinterpret_cast<void (*)()>(+arg);
    return rv;
  }
  template <typename State>
  static auto is_null(State arg) noexcept -> bool {
    return arg.fn == nullptr;
  }

  template <typename State, typename Fn, typename Ret, typename... Args>
  static auto call(State state, Args... args) noexcept(
      __VEG_CONCEPT(meta::nothrow_invocable<Fn&&, Args&&...>)) -> Ret {
    return discard_void<Ret>::apply(
        reinterpret_cast<decltype(+__VEG_DECLVAL(Fn&))>(state.fn),
        VEG_FWD(args)...);
  }
};

template <>
struct fn_view_impl<fn_kind_e::mem_fn_ptr> {
  template <typename State, typename Fn>
  static auto address(Fn& arg) noexcept -> state_t {
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
  static auto is_null(State arg) noexcept -> bool {
    return arg.mem_fn == nullptr;
  }

  template <typename State, typename Fn, typename Ret, typename... Args>
  static auto call(State state, Args... args) noexcept(
      __VEG_CONCEPT(meta::nothrow_invocable<Fn&&, Args&&...>)) -> Ret {
    return discard_void<Ret>::apply(
        reinterpret_cast<decltype(__VEG_DECLVAL(Fn&))>(state.mem_fn),
        VEG_FWD(args)...);
  }
};

template <bool No_Except, typename State, typename Ret, typename... Args>
struct function_ref_impl {

  constexpr function_ref_impl() = default;
  // COMPAT: check if function_ref_impl is a base of Fn
  template <typename Fn>
  function_ref_impl(char /*unused*/, Fn&& fn) noexcept
      : m_state(fnref::fn_view_impl<fnref::fn_kind<meta::decay_t<Fn>>::value>::
                    template address<State>(fn)),
        m_call(fnref::fn_view_impl<fnref::fn_kind<meta::decay_t<Fn>>::value>::
                   template call<State, Fn, Ret, Args...>) {
    VEG_ASSERT(
        !fnref::fn_view_impl<fnref::fn_kind<meta::decay_t<Fn>>::value>::is_null(
            m_state));
  }

  function_ref_impl(
      State state,
      auto (*call)(State, Args...) noexcept(No_Except)->Ret) noexcept
      : m_state(state), m_call(call) {}

  auto call_fn(Args... args) const noexcept(No_Except) -> Ret {
    VEG_DEBUG_ASSERT(m_call != nullptr);
    return this->m_call(this->m_state, VEG_FWD(args)...);
  }

  template <fnref::fn_kind_e>
  friend struct fnref::fn_view_impl;

  State m_state = {nullptr};
  auto (*m_call)(State, Args...) noexcept(No_Except) -> Ret = nullptr;
};

} // namespace fnref
} // namespace internal

template <typename Ret, typename... Args>
struct fn_view<Ret(Args...)>
    : private internal::fnref::
          function_ref_impl<false, internal::fnref::state_t, Ret, Args...> {
private:
  using base = internal::fnref::
      function_ref_impl<false, internal::fnref::state_t, Ret, Args...>;
  explicit constexpr fn_view(internal::fnref::dummy /*unused*/) noexcept
      : base(){};
  template <typename T>
  friend struct meta::value_sentinel_for;
  template <typename T>
  friend struct fn_view;
  template <typename T>
  friend struct once_fn_view;

public:
  fn_view() = delete;

  VEG_TEMPLATE(
      (typename Fn),
      requires(
          !__VEG_CONCEPT(meta::base_of<meta::remove_cvref_t<Fn>, fn_view>) &&
          __VEG_CONCEPT(meta::invocable<Fn&, Args&&...>) &&
          (__VEG_CONCEPT(meta::void_<Ret>) ||
           __VEG_SAME_AS(Ret, (meta::invoke_result_t<Fn&, Args&&...>)))),
      fn_view, // NOLINT
      (fn, Fn&&))
  noexcept
      : base({}, fn) {}

  __VEG_CPP17(

      fn_view /* NOLINT */ (fn_view<Ret(Args...) noexcept> fn) noexcept
      : base(fn.m_state, fn.m_call){}

  )

  auto operator()(Args... args) const -> Ret {
    return this->call_fn(VEG_FWD(args)...);
  }
};

__VEG_CPP17(

 template <typename Ret, typename... Args>
 struct fn_view<Ret(Args...) noexcept>
 : private internal::fnref::
 function_ref_impl<true, internal::fnref::state_t, Ret, Args...> {
 private:
 using base = internal::fnref::
 function_ref_impl<true, internal::fnref::state_t, Ret, Args...>;
 explicit constexpr fn_view(internal::fnref::dummy /*unused*/) noexcept
 : base(){};
 template <typename T>
 friend struct meta::value_sentinel_for;
 template <typename T>
 friend struct fn_view;
 template <typename T>
 friend struct once_fn_view;

 public:
 fn_view() = delete;

 VEG_TEMPLATE(
     (typename Fn),
     requires(
       !__VEG_CONCEPT(meta::base_of<meta::remove_cvref_t<Fn>, fn_view>) &&
       __VEG_CONCEPT(meta::nothrow_invocable<Fn&, Args&&...>) &&
       (__VEG_CONCEPT(meta::void_<Ret>) ||
        __VEG_SAME_AS(Ret, (meta::invoke_result_t<Fn&, Args&&...>)))),
     fn_view, // NOLINT
     (fn, Fn&&))
   noexcept
   : base({}, fn) {}

 auto operator()(Args... args) const noexcept -> Ret {
   return this->call_fn(VEG_FWD(args)...);
 }
 };

)

template <typename Ret, typename... Args>
struct once_fn_view<Ret(Args...)>
    : private internal::fnref::
          function_ref_impl<false, internal::fnref::state_t, Ret, Args...>,
      meta::internal::delete_copy {
private:
  using base = internal::fnref::
      function_ref_impl<false, internal::fnref::state_t, Ret, Args...>;
  explicit constexpr once_fn_view(internal::fnref::dummy /*unused*/) noexcept
      : base(){};
  template <typename T>
  friend struct meta::value_sentinel_for;
  template <typename T>
  friend struct fn_view;
  template <typename T>
  friend struct once_fn_view;

public:
  once_fn_view() = delete;

  VEG_TEMPLATE(
      (typename Fn),
      requires(
          !__VEG_CONCEPT(
              meta::base_of<meta::remove_cvref_t<Fn>, once_fn_view>) &&
          __VEG_CONCEPT(meta::invocable<Fn&&, Args&&...>) &&
          (__VEG_CONCEPT(meta::void_<Ret>) ||
           __VEG_SAME_AS(Ret, (meta::invoke_result_t<Fn&&, Args&&...>)))),
      once_fn_view, // NOLINT
      (fn, Fn&&))
  noexcept
      : base({}, VEG_FWD(fn)) {}

  __VEG_CPP17(

      once_fn_view /* NOLINT */ (fn_view<Ret(Args...) noexcept> fn) noexcept
      : base(fn.m_state, fn.m_call){}

  )
  once_fn_view /* NOLINT */ (fn_view<Ret(Args...)> fn) noexcept
      : base(fn.m_state, fn.m_call) {}

  auto operator()(Args... args) const&& -> Ret {
    return this->call_fn(VEG_FWD(args)...);
  }
};

__VEG_CPP17(
 template <typename Ret, typename... Args>
 struct once_fn_view<Ret(Args...) noexcept>
 : private internal::fnref::
 function_ref_impl<true, internal::fnref::state_t, Ret, Args...>,
 meta::internal::delete_copy {
 private:
 using base = internal::fnref::
 function_ref_impl<true, internal::fnref::state_t, Ret, Args...>;
 explicit constexpr once_fn_view(internal::fnref::dummy /*unused*/) noexcept
 : base(){};
 template <typename T>
 friend struct meta::value_sentinel_for;
 template <typename T>
 friend struct fn_view;
 template <typename T>
 friend struct once_fn_view;

 public:
 once_fn_view() = delete;

 VEG_TEMPLATE(
     (typename Fn),
     requires(
       !__VEG_CONCEPT(
         meta::base_of<meta::remove_cvref_t<Fn>, once_fn_view>) &&
       __VEG_CONCEPT(meta::nothrow_invocable<Fn&&, Args&&...>) &&
       (__VEG_CONCEPT(meta::void_<Ret>) ||
        __VEG_SAME_AS(Ret, (meta::invoke_result_t<Fn&&, Args&&...>)))),
     once_fn_view, // NOLINT
     (fn, Fn&&))
   noexcept
   : base({}, VEG_FWD(fn)) {}

 once_fn_view /* NOLINT */ (fn_view<Ret(Args...) noexcept> fn) noexcept
   : base(fn.m_state, fn.m_call) {}

 auto operator()(Args... args) const&& noexcept -> Ret {
   return this->call_fn(VEG_FWD(args)...);
 }
 };
)

template <typename T>
struct meta::value_sentinel_for<fn_view<T>> : std::integral_constant<i64, 1> {
  static constexpr auto invalid(i64 i) noexcept -> fn_view<T> {
    return VEG_ASSERT(i == 0), fn_view<T>{::veg::internal::fnref::dummy{}};
  }
  static constexpr auto id(fn_view<T> arg) -> i64 {
    return arg.m_call == nullptr ? 0 : -1;
  }
};
template <typename T>
struct meta::value_sentinel_for<once_fn_view<T>>
    : std::integral_constant<i64, 1> {
  static constexpr auto invalid(i64 i) noexcept -> once_fn_view<T> {
    return VEG_ASSERT(i == 0), once_fn_view<T>{::veg::internal::fnref::dummy{}};
  }
  static constexpr auto id(once_fn_view<T> arg) -> i64 {
    return arg.m_call == nullptr ? 0 : -1;
  }
};

} // namespace veg

#endif /* end of include guard __VEG_FN_REF_HPP_IDVWA3EYS */
