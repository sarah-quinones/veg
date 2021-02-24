#ifndef __VEG_OPTION_HPP_8NVLXES2S
#define __VEG_OPTION_HPP_8NVLXES2S

#include "veg/assert.hpp"
#include "veg/internal/memory.hpp"
#include "veg/internal/storage.hpp"

namespace veg {

template <typename T>
struct option;
namespace meta {
template <typename T>
struct mostly_trivial<option<T>> : mostly_trivial<T> {};
template <typename T>
struct trivially_relocatable<option<T>> : trivially_relocatable<T> {};
template <typename T>
struct is_option : std::false_type {};
template <typename T>
struct is_option<veg::option<T>> : std::true_type {};

template <typename T>
struct option_inner {};
template <typename T>
struct option_inner<option<T>> {
  using type = T;
};
} // namespace meta
struct none_t {
  friend constexpr auto operator==(none_t /*lhs*/, none_t /*rhs*/) noexcept
      -> bool {
    return true;
  }
  friend constexpr auto operator!=(none_t /*lhs*/, none_t /*rhs*/) noexcept
      -> bool {
    return false;
  }

private:
  constexpr none_t() = default;
  constexpr explicit none_t(none_t* /*unused*/) noexcept {}
  template <typename T>
  friend struct meta::internal::static_const;
};
__VEG_ODR_VAR(none, none_t);
struct some_t {
  VEG_TEMPLATE(
      (typename T),
      requires(meta::constructible<meta::remove_cvref_t<T>, T&&>::value),
      __VEG_CPP14(constexpr) auto
      operator(),
      (arg, T&&))
  const noexcept(
      meta::nothrow_constructible<meta::remove_cvref_t<T>, T&&>::value)
      ->option<meta::remove_cvref_t<T>> {
    return {*this, VEG_FWD(arg)};
  }

private:
  constexpr some_t() = default;
  constexpr explicit some_t(some_t* /*unused*/) noexcept {}
  template <typename T>
  friend struct meta::internal::static_const;
};
__VEG_ODR_VAR(some, some_t);
struct some_ref_t {
  template <typename T>
  __VEG_CPP14(constexpr)
  auto operator()(T&& arg) const noexcept -> option<T&&> {
    return {some, VEG_FWD(arg)};
  }

private:
  constexpr some_ref_t() = default;
  constexpr explicit some_ref_t(some_ref_t* /*unused*/) noexcept {}
  template <typename T>
  friend struct meta::internal::static_const;
};
__VEG_ODR_VAR(some_ref, some_ref_t);

namespace internal {
namespace option_ {
struct hidden_tag {};

enum kind { has_sentinel, trivial, non_trivial };

template <typename T, bool = (meta::value_sentinel_for<T>::value > 1)>
struct value_sentinel_impl
    : std::integral_constant<i64, i64(static_cast<unsigned char>(-3))> {
  static __VEG_CPP14(constexpr) auto invalid(i64 i) noexcept -> option<T> {
    if (i <= static_cast<unsigned char>(-3)) {
      option<T> val = none;
      val.engaged = static_cast<unsigned char>(2 + i);
      return val;
    }
    terminate();
  }
  static __VEG_CPP14(constexpr) auto id(option<T> const& arg) noexcept -> i64 {
    return (arg.engaged < 2) ? -1 : (arg.engaged - 2);
  }
};

template <typename T>
struct value_sentinel_impl<T, true>
    : std::integral_constant<i64, meta::value_sentinel_for<T>::value - 1> {
  static __VEG_CPP14(constexpr) auto invalid(i64 i) noexcept -> option<T> {
    return option<T>{meta::value_sentinel_for<T>::invalid(i + 1)};
  }
  static __VEG_CPP14(constexpr) auto id(option<T> const& arg) noexcept -> i64 {
    if (arg) {
      return -1;
    }
    return meta::value_sentinel_for<T>::id(
        arg.unwrap_unchecked_invalid(unsafe));
  }
};

template <typename T, typename Self>
__VEG_CPP14(constexpr)
auto as_ref_impl(Self&& self) noexcept
    -> option<meta::collapse_category_t<T, Self&&>> {
  if (self) {
    return {
        some,
        static_cast<meta::collapse_category_t<option<T>, Self&&>>(VEG_FWD(self))
            ._get()};
  }
  return none;
}

template <typename Fn>
struct do_if {
  bool cond;
  Fn fn;
  void operator()() const noexcept {
    if (cond) {
      fn();
    }
  }
};

struct make_none { // NOLINT
  unsigned char* c;
  __VEG_CPP20(constexpr) void operator()() const noexcept {
    mem::construct_at(c);
  }
};

template <typename T>
using kind_v = std::integral_constant<
    kind,
    ((meta::value_sentinel_for<storage::storage<T>>::value > 0) //
         ? has_sentinel
         : ((meta::mostly_trivial<T>::value &&     //
             meta::copy_assignable<T>::value &&    //
             meta::move_assignable<T>::value &&    //
             meta::constructible<T>::value &&      //
             meta::copy_constructible<T>::value && //
             meta::move_constructible<T>::value))
               ? trivial
               : non_trivial)>;

template <typename T, kind = kind_v<T>::value>
// trivial
struct option_storage_base {
  static_assert(
      meta::trivially_copyable<T>::value,
      "mostly_trivial is probably mis-defined");

  storage::storage<T> some = {};
  unsigned char engaged = 0;

  __VEG_CPP14(constexpr) option_storage_base() = default;
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) explicit option_storage_base(
      T&& val) noexcept(meta::nothrow_move_constructible<T>::value)
      : some(VEG_FWD(val)), engaged(1) {}

  template <typename Fn, typename... Args>
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) explicit option_storage_base(
      hidden_tag /*unused*/, Fn&& fn, Args&&... args) //
      noexcept(meta::nothrow_invocable<Fn&&, Args&&...>::value)
      : some(0, VEG_FWD(fn), VEG_FWD(args)...), engaged(1) {}

  VEG_NODISCARD HEDLEY_ALWAYS_INLINE
  __VEG_CPP14(constexpr) auto is_engaged() const noexcept -> bool {
    return engaged == 1;
  }

  template <typename Fn>
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) void _emplace(Fn&& fn) noexcept(
      meta::nothrow_invocable<Fn&&>::value) {
    VEG_DEBUG_ASSERT(!engaged);
    some = storage::storage<T>(veg::invoke(VEG_FWD(fn)));
    engaged = 1;
  }

  template <typename U>
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) void assign(U&& rhs) noexcept(
      noexcept(some.assign(VEG_FWD(rhs)))) {
    VEG_DEBUG_ASSERT(engaged);
    some.assign(VEG_FWD(rhs));
  }

  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) void destroy() noexcept {
    VEG_DEBUG_ASSERT(engaged);
    engaged = 0;
  }

  VEG_NODISCARD HEDLEY_ALWAYS_INLINE
  __VEG_CPP14(constexpr) auto _get() const& noexcept -> T const& {
    VEG_DEBUG_ASSERT(is_engaged());
    return some._get();
  }
  VEG_NODISCARD HEDLEY_ALWAYS_INLINE
  __VEG_CPP14(constexpr) auto _get() & noexcept -> T& {
    VEG_DEBUG_ASSERT(is_engaged());
    return some.get_mut();
  }
  VEG_NODISCARD HEDLEY_ALWAYS_INLINE
  __VEG_CPP14(constexpr) auto _get() && noexcept -> T&& {
    VEG_DEBUG_ASSERT(is_engaged());
    return VEG_FWD(some).get_mov_ref();
  }
};

template <typename T>
struct option_storage_base<T, has_sentinel> {
  using sentinel_traits = meta::value_sentinel_for<storage::storage<T>>;

  static_assert(meta::trivially_destructible<T>::value, "um");
  static_assert(meta::trivially_copyable<storage::storage<T>>::value, "err");

  storage::storage<T> some = sentinel_traits::invalid(0);

  __VEG_CPP14(constexpr) option_storage_base() = default;
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) explicit option_storage_base(
      T&& val) noexcept(meta::nothrow_move_constructible<T>::value)
      : some(VEG_FWD(val)) {}

  template <typename Fn, typename... Args>
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) explicit option_storage_base(
      hidden_tag /*unused*/, Fn&& fn, Args&&... args) //
      noexcept(meta::nothrow_invocable<Fn&&, Args&&...>::value)
      : some(0, VEG_FWD(fn), VEG_FWD(args)...) {}

  VEG_NODISCARD HEDLEY_ALWAYS_INLINE
  __VEG_CPP14(constexpr) auto is_engaged() const noexcept -> bool {
    return sentinel_traits::id(some) < 0;
  }

  template <typename Fn>
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) void _emplace(Fn&& fn) noexcept(
      meta::nothrow_invocable<Fn&&>::value) {
    VEG_DEBUG_ASSERT(!is_engaged());
    some = storage::storage<T>(veg::invoke(VEG_FWD(fn)));
  }

  template <typename U>
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) void assign(U&& rhs) noexcept(
      noexcept(some.assign(VEG_FWD(rhs)))) {
    VEG_DEBUG_ASSERT(is_engaged());
    some.assign(VEG_FWD(rhs));
  }

  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) void destroy() noexcept {
    some = sentinel_traits::invalid(0);
  }

  VEG_NODISCARD HEDLEY_ALWAYS_INLINE __VEG_CPP14(
      constexpr) auto unwrap_unchecked_invalid(unsafe_t /*tag*/) const& noexcept
      -> T const& {
    return some._get();
  }

  VEG_NODISCARD HEDLEY_ALWAYS_INLINE
  __VEG_CPP14(constexpr) auto _get() const& noexcept -> T const& {
    VEG_DEBUG_ASSERT(is_engaged());
    return some._get();
  }
  VEG_NODISCARD HEDLEY_ALWAYS_INLINE
  __VEG_CPP14(constexpr) auto _get() & noexcept -> T& {
    VEG_DEBUG_ASSERT(is_engaged());
    return some.get_mut();
  }
  VEG_NODISCARD HEDLEY_ALWAYS_INLINE
  __VEG_CPP14(constexpr) auto _get() && noexcept -> T&& {
    VEG_DEBUG_ASSERT(is_engaged());
    return VEG_FWD(some).get_mov_ref();
  }
};

template <typename T, bool = meta::mostly_trivial<T>::value>
struct option_storage_base_option_emplace {
  template <typename Fn>
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) void _emplace(Fn&& fn) noexcept(
      meta::nothrow_invocable<Fn&&>::value) {
    auto& self = static_cast<option_storage_base<option<T>>&>(*this);
    VEG_DEBUG_ASSERT(!self.is_engaged());
    self.some = VEG_FWD(fn)();
  }
};

template <typename T>
struct option_storage_base_option_emplace<T, false> {
  template <typename Fn>
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) void _emplace(Fn&& fn) noexcept(
      meta::nothrow_invocable<Fn&&>::value) {
    auto& self = *static_cast<option_storage_base<option<T>>*>(this);
    VEG_DEBUG_ASSERT(!self.is_engaged());
    // mem::destroy_at(mem::addressof(some)); // no-op
    mem::construct_with(mem::addressof(self.some), VEG_FWD(fn));
  }
};

template <typename T>
struct option_storage_base<option<T>, has_sentinel>
    : option_storage_base_option_emplace<T> {
  using sentinel_traits = meta::value_sentinel_for<option<T>>;

  option<T> some = sentinel_traits::invalid(0);

  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) option_storage_base() = default;
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) explicit option_storage_base(
      option<T>&& val) noexcept(meta::nothrow_move_constructible<T>::value)
      : some(VEG_FWD(val)) {}

  template <typename Fn, typename... Args>
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) explicit option_storage_base(
      hidden_tag /*unused*/, Fn&& fn, Args&&... args) //
      noexcept(meta::nothrow_invocable<Fn&&, Args&&...>::value)
      : some(invoke(VEG_FWD(fn), VEG_FWD(args)...)) {}

  VEG_NODISCARD __VEG_CPP14(constexpr) auto is_engaged() const noexcept
      -> bool {
    return sentinel_traits::id(some) < 0;
  }

  template <typename U>
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) void assign(U&& rhs) noexcept(
      noexcept(some.assign(VEG_FWD(rhs)))) {
    VEG_DEBUG_ASSERT(is_engaged());
    some = VEG_FWD(rhs);
  }

  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) void destroy() noexcept {
    some = sentinel_traits::invalid(0);
  }

  VEG_NODISCARD HEDLEY_ALWAYS_INLINE __VEG_CPP14(
      constexpr) auto unwrap_unchecked_invalid(unsafe_t /*tag*/) const& noexcept
      -> option<T> const& {
    return some;
  }

  VEG_NODISCARD HEDLEY_ALWAYS_INLINE
  __VEG_CPP14(constexpr) auto _get() const& noexcept -> option<T> const& {
    VEG_DEBUG_ASSERT(is_engaged());
    return some;
  }
  VEG_NODISCARD HEDLEY_ALWAYS_INLINE
  __VEG_CPP14(constexpr) auto _get() & noexcept -> option<T>& {
    VEG_DEBUG_ASSERT(is_engaged());
    return some;
  }
  VEG_NODISCARD HEDLEY_ALWAYS_INLINE
  __VEG_CPP14(constexpr) auto _get() && noexcept -> option<T>&& {
    VEG_DEBUG_ASSERT(is_engaged());
    return VEG_FWD(some);
  }
};

template <typename T, bool = meta::trivially_destructible<T>::value>
struct option_storage_nontrivial_base {
  union {
    unsigned char none = {};
    storage::storage<T> some;
  };
  unsigned char engaged = 0;

  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr)
      option_storage_nontrivial_base() noexcept {};
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) option_storage_nontrivial_base(
      i64 /* unused */, unsigned char id) noexcept
      : engaged{id} {};

  HEDLEY_ALWAYS_INLINE
  __VEG_CPP14(constexpr)
  explicit option_storage_nontrivial_base(T&& val) //
      noexcept(meta::nothrow_move_constructible<T>::value)
      : some(VEG_FWD(val)), engaged(1) {}

  template <typename Fn, typename... Args>
  HEDLEY_ALWAYS_INLINE
  __VEG_CPP14(constexpr) explicit option_storage_nontrivial_base(
      hidden_tag /*unused*/, Fn&& fn, Args&&... args) //
      noexcept(meta::nothrow_invocable<Fn&&, Args&&...>::value)
      : some(0, VEG_FWD(fn), VEG_FWD(args)...), engaged(1) {}

  VEG_NODISCARD HEDLEY_ALWAYS_INLINE
  __VEG_CPP14(constexpr) auto is_engaged() const noexcept -> bool {
    return engaged == 1;
  }
  __VEG_CPP14(constexpr)
  HEDLEY_ALWAYS_INLINE void set_engaged(bool b) noexcept {
    engaged = static_cast<unsigned char>(b);
  }
};

template <typename T>
struct
    option_storage_nontrivial_base // NOLINT(cppcoreguidelines-special-member-functions)
    <T, false> {
  union {
    unsigned char none = {};
    storage::storage<T> some;
  };
  unsigned char engaged = 0;
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr)
      option_storage_nontrivial_base() noexcept {};
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) option_storage_nontrivial_base(
      i64 /* unused */, unsigned char id) noexcept
      : engaged{id} {};
  HEDLEY_ALWAYS_INLINE
  __VEG_CPP14(constexpr)
  explicit option_storage_nontrivial_base(T&& val) //
      noexcept(meta::nothrow_move_constructible<T>::value)
      : some(VEG_FWD(val)), engaged(1) {}

  template <typename Fn, typename... Args>
  HEDLEY_ALWAYS_INLINE
  __VEG_CPP14(constexpr) explicit option_storage_nontrivial_base(
      hidden_tag /*unused*/, Fn&& fn, Args&&... args) //
      noexcept(meta::nothrow_invocable<Fn&&, Args&&...>::value)
      : some(0, VEG_FWD(fn), VEG_FWD(args)...), engaged(1) {}

  VEG_NODISCARD HEDLEY_ALWAYS_INLINE
  __VEG_CPP14(constexpr) auto is_engaged() const noexcept -> bool {
    return engaged == 1;
  }
  HEDLEY_ALWAYS_INLINE
  __VEG_CPP14(constexpr) void set_engaged(bool b) noexcept {
    engaged = static_cast<unsigned char>(b);
  }

  HEDLEY_ALWAYS_INLINE
  __VEG_CPP20(constexpr) ~option_storage_nontrivial_base() {
    if (engaged == 1) {
      mem::destroy_at(mem::addressof(some));
    } else {
      mem::destroy_at(mem::addressof(none));
    }
  }
};

template <typename T>
struct option_storage_base<T, non_trivial> : option_storage_nontrivial_base<T> {

  using option_storage_nontrivial_base<T>::option_storage_nontrivial_base;
  using option_storage_nontrivial_base<T>::some;
  using option_storage_nontrivial_base<T>::none;
  using option_storage_nontrivial_base<T>::is_engaged;
  using option_storage_nontrivial_base<T>::set_engaged;

  template <typename U>
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) void assign(U&& rhs) noexcept(
      noexcept(some.assign(VEG_FWD(rhs)))) {
    VEG_DEBUG_ASSERT(is_engaged());
    some.assign(VEG_FWD(rhs));
  }

  template <typename Fn>
  __VEG_CPP20(constexpr)
  HEDLEY_ALWAYS_INLINE
      void _emplace(Fn&& fn) noexcept(meta::nothrow_invocable<Fn&&>::value) {
    VEG_DEBUG_ASSERT(!is_engaged());

    defer<do_if<make_none>> guard{{true, {mem::addressof(none)}}};

    mem::construct_at(mem::addressof(some), 0, VEG_FWD(fn));

    guard.fn.cond = false;
    set_engaged(true);
  }

  HEDLEY_ALWAYS_INLINE __VEG_CPP20(constexpr) void destroy() noexcept {
    VEG_DEBUG_ASSERT(is_engaged());
    set_engaged(false);
    defer<make_none> guard{mem::addressof(none)};
    mem::destroy_at(mem::addressof(some));
  }

  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto _get() const& noexcept
      -> T const& {
    VEG_DEBUG_ASSERT(is_engaged());
    return some._get();
  }
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto _get() & noexcept -> T& {
    VEG_DEBUG_ASSERT(is_engaged());
    return some.get_mut();
  }
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto _get() && noexcept -> T&& {
    VEG_DEBUG_ASSERT(is_engaged());
    return VEG_FWD(some).get_mov_ref();
  }
};

template <
    typename T,
    bool = (kind_v<T>::value != non_trivial) || meta::reference<T>::value ||
           meta::trivially_copy_constructible<T>::value ||
           !meta::constructible<T, T const&>::value>
struct option_copy_ctor_base : option_storage_base<T> {
  using option_storage_base<T>::option_storage_base;
};
template <typename T>
struct option_copy_ctor_base<T, false> : option_storage_base<T> {
  using option_storage_base<T>::option_storage_base;

  ~option_copy_ctor_base() = default;
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr)
      option_copy_ctor_base(option_copy_ctor_base const& rhs) noexcept(
          meta::nothrow_constructible<T, T const&>::value)
      : option_storage_base<T>{0, rhs.engaged} {
    if (rhs.is_engaged()) {
      this->engaged = 0;
      this->_emplace(storage::copy_ctor_fn<T>{rhs._get()});
    }
  }
  option_copy_ctor_base /* NOLINT */ (option_copy_ctor_base&&) = default;
  auto operator=(option_copy_ctor_base const&)
      -> option_copy_ctor_base& = default;
  auto operator= /* NOLINT */(option_copy_ctor_base&&)
      -> option_copy_ctor_base& = default;
};

template <
    typename T,
    bool = (kind_v<T>::value != non_trivial) || meta::reference<T>::value ||
           meta::trivially_move_constructible<T>::value ||
           !meta::constructible<T, T&&>::value>
struct option_move_ctor_base : option_copy_ctor_base<T> {
  using option_copy_ctor_base<T>::option_copy_ctor_base;
};

template <typename T>
struct option_move_ctor_base<T, false> : option_copy_ctor_base<T> {
  using option_copy_ctor_base<T>::option_copy_ctor_base;

  ~option_move_ctor_base() = default;
  option_move_ctor_base(option_move_ctor_base const&) = default;
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr)
      option_move_ctor_base(option_move_ctor_base&& rhs) noexcept(
          meta::nothrow_constructible<T, T&&>::value)
      : option_copy_ctor_base<T>{0, rhs.engaged} {
    if (rhs.is_engaged()) {
      this->engaged = 0;
      this->_emplace(storage::move_ctor_fn<T>{VEG_FWD(rhs)._get()});
    }
  }
  auto operator=(option_move_ctor_base const&)
      -> option_move_ctor_base& = default;
  auto operator= /* NOLINT */(option_move_ctor_base&&)
      -> option_move_ctor_base& = default;
};

template <
    typename T,
    bool = (kind_v<T>::value != non_trivial) || meta::reference<T>::value ||
           meta::trivially_copy_assignable<T>::value ||
           !meta::assignable<T, T const&>::value>
struct option_copy_assign_base : option_move_ctor_base<T> {
  using option_move_ctor_base<T>::option_move_ctor_base;
};
template <typename T>
struct option_copy_assign_base<T, false> : option_move_ctor_base<T> {
  using option_move_ctor_base<T>::option_move_ctor_base;
  ~option_copy_assign_base() = default;

  option_copy_assign_base(option_copy_assign_base const&) = default;
  option_copy_assign_base /* NOLINT */ (option_copy_assign_base&&) = default;
  __VEG_CPP14(constexpr)
  auto operator= // NOLINT(cert-oop54-cpp)
      (option_copy_assign_base const& rhs) noexcept(
          (meta::nothrow_constructible<T, T const&>::value &&
           meta::nothrow_assignable<T&, T const&>::value))
          -> option_copy_assign_base& {
    if (rhs.is_engaged()) {
      if (this->is_engaged()) {
        this->assign(rhs._get());
      } else {
        this->_emplace(storage::copy_ctor_fn<T>{rhs._get()});
      }
    } else {
      if (this->is_engaged()) {
        this->destroy();
      }
      this->engaged = rhs.engaged;
    }
    return *this;
  }
  auto operator= /* NOLINT */(option_copy_assign_base&&)
      -> option_copy_assign_base& = default;
};

template <
    typename T,
    bool = (kind_v<T>::value != non_trivial) || meta::reference<T>::value ||
           meta::trivially_move_assignable<T>::value ||
           !meta::assignable<T, T&&>::value>
struct option_move_assign_base : option_copy_assign_base<T> {
  using option_copy_assign_base<T>::option_copy_assign_base;
};
template <typename T>
struct option_move_assign_base<T, false> : option_copy_assign_base<T> {
  using option_copy_assign_base<T>::option_copy_assign_base;

  ~option_move_assign_base() = default;
  option_move_assign_base(option_move_assign_base const&) = default;
  option_move_assign_base /* NOLINT */ (option_move_assign_base&&) = default;
  auto operator=(option_move_assign_base const&)
      -> option_move_assign_base& = default;
  __VEG_CPP14(constexpr)
  auto operator=(option_move_assign_base&& rhs) noexcept(
      (meta::nothrow_constructible<T, T&&>::value &&
       meta::nothrow_assignable<T&, T&&>::value)) -> option_move_assign_base& {
    if (rhs.is_engaged()) {
      if (this->is_engaged()) {
        this->assign(VEG_FWD(rhs)._get());
      } else {
        this->_emplace(storage::move_ctor_fn<T>{VEG_FWD(rhs)._get()});
      }
    } else {
      if (this->is_engaged()) {
        this->destroy();
      }
      this->engaged = rhs.engaged;
    }
    return *this;
  }
};

template <typename T, bool = meta::move_constructible<T>::value>
struct option_move_ctor_base2 : option_move_assign_base<T> {
  option_move_ctor_base2() = default;
  explicit __VEG_CPP14(constexpr) option_move_ctor_base2(T value) noexcept(
      meta::nothrow_constructible<T, T&&>::value)
      : internal::option_::option_move_assign_base<T>(VEG_FWD(value)) {}

  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr)
      option_move_ctor_base2(some_t /*tag*/, T value) noexcept(
          meta::nothrow_constructible<T, T&&>::value)
      : internal::option_::option_move_assign_base<T>(VEG_FWD(value)) {}

  VEG_TEMPLATE(
      (typename Fn, typename... Args),
      requires __VEG_SAME_AS(
          T, (meta::detected_t<meta::invoke_result_t, Fn&&, Args&&...>)),
      HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) option_move_ctor_base2,

      (/*tag*/, some_t),
      (/*tag*/, inplace_t),
      (fn, Fn&&),
      (... args, Args&&))
  noexcept(meta::nothrow_invocable<Fn&&, Args&&...>::value)
      : internal::option_::option_move_assign_base<T>(
            internal::option_::hidden_tag{}, VEG_FWD(fn), VEG_FWD(args)...) {}
};

template <typename T>
struct option_move_ctor_base2<T, false> : option_move_assign_base<T> {
  option_move_ctor_base2() = default;
  VEG_TEMPLATE(
      (typename Fn, typename... Args),
      requires __VEG_SAME_AS(
          T, (meta::detected_t<meta::invoke_result_t, Fn&&, Args&&...>)),
      HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) option_move_ctor_base2,

      (/*tag*/, some_t),
      (/*tag*/, inplace_t),
      (fn, Fn&&),
      (... args, Args&&))
  noexcept(meta::nothrow_invocable<Fn&&, Args&&...>::value)
      : internal::option_::option_move_assign_base<T>(
            internal::option_::hidden_tag{}, VEG_FWD(fn), VEG_FWD(args)...) {}
};

template <typename To>
struct decay_fn {
  template <typename T>
  HEDLEY_ALWAYS_INLINE constexpr auto operator()(T&& ref) const
      noexcept(meta::nothrow_constructible<To, T&&>::value) -> To {
    return VEG_FWD(ref);
  }
};

template <typename T, bool = meta::move_constructible<T>::value>
struct option_move_interface_base {
  __VEG_CPP14(constexpr)
  auto clone() && noexcept(meta::nothrow_move_constructible<T>::value)
      -> option<T> {
    auto&& self = static_cast<option<T>&&>(*this);
    return VEG_FWD(self).as_ref().map(decay_fn<T>{});
  }

  VEG_NODISCARD
  __VEG_CPP14(constexpr)
  auto take() noexcept(meta::move_constructible<T>::value) -> option<T> {
    auto& self = static_cast<option<T>&>(*this);
    if (self) {
      T val = static_cast<option<T>&&>(self)._get();
      self = none;
      return {some, VEG_FWD(val)};
    }
    return none;
  }

  VEG_NODISCARD HEDLEY_ALWAYS_INLINE
  __VEG_CPP14(constexpr) auto unwrap_unchecked(unsafe_t /*tag*/) && noexcept(
      meta::nothrow_move_constructible<T>::value) -> T {
    auto&& self = static_cast<option<T>&&>(*this);
    return VEG_FWD(self)._get();
  }

  VEG_NODISCARD __VEG_CPP14(constexpr) auto unwrap() && noexcept(
      meta::nothrow_move_constructible<T>::value) -> T {
    auto& self = static_cast<option<T>&>(*this);
    VEG_ASSERT(self);
    return static_cast<option<T>&&>(self).unwrap_unchecked(unsafe);
  }

  VEG_TEMPLATE(
      (typename Fn),
      requires(meta::constructible<
               bool,
               meta::detected_t<
                   meta::invoke_result_t,
                   Fn,
                   meta::remove_cvref_t<T> const&>>::value),
      VEG_NODISCARD __VEG_CPP14(constexpr) auto filter,
      (fn, Fn&&)) &&

      noexcept(
          (meta::nothrow_invocable<Fn&&, meta::remove_cvref_t<T> const&>::
               value &&
           meta::nothrow_move_constructible<T>::value)) -> option<T> {
    auto&& self = static_cast<option<T>&&>(*this);
    if (self) {
      if (invoke(VEG_FWD(fn), self.as_cref().unwrap_unchecked(unsafe))) {
        return {some, VEG_FWD(self).as_ref().unwrap_unchecked(unsafe)};
      }
    }
    return none;
  }
};

template <typename T, bool = meta::copy_constructible<T>::value>
struct option_copy_interface_base {

  __VEG_CPP14(constexpr)
  auto clone() const& noexcept(meta::nothrow_constructible<T, T const&>::value)
      -> option<T> {
    auto const& self = static_cast<option<T> const&>(*this);
    return self.as_ref().map(decay_fn<T>{});
  }
};

template <typename T>
struct option_copy_interface_base<T, false> {};
template <typename T>
struct option_move_interface_base<T, false> {};

template <
    typename T,
    bool B1 = meta::copy_constructible<T>::value,
    bool B2 = meta::move_constructible<T>::value>
struct option_copymove_interface_base : option_copy_interface_base<T, B1>,
                                        option_move_interface_base<T, B2> {};

template <typename T>
struct option_copymove_interface_base<T, true, true>
    : option_copy_interface_base<T, true>, option_move_interface_base<T, true> {
  using option_copy_interface_base<T, true>::clone;
  using option_move_interface_base<T, true>::clone;
};

template <typename T, typename U, typename Fn>
HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto cmp(
    option<T> const& lhs, option<U> const& rhs, Fn fn) noexcept -> bool {
  if (lhs) {
    if (rhs) {
      return static_cast<bool>(
          fn(lhs.as_cref().unwrap_unchecked(unsafe),
             rhs.as_cref().unwrap_unchecked(unsafe)));
    }
  }
  return (static_cast<bool>(lhs) == static_cast<bool>(rhs));
}

#define __VEG_CMP(op, fn, ...)                                                 \
  VEG_TEMPLATE(                                                                \
      (typename T, typename U),                                                \
      requires(__VA_ARGS__),                                                   \
      VEG_NODISCARD HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto           \
      operator op,                                                             \
      (lhs, option<T> const&),                                                 \
      (rhs, option<U> const&))                                                 \
  noexcept->bool {                                                             \
    return option_::cmp(lhs, rhs, fn);                                         \
  }                                                                            \
  static_assert(true, "")

__VEG_CMP(==, cmp_equal, meta::equality_comparable_with<T, U>::value);
__VEG_CMP(<, cmp_less, meta::partially_ordered_with<T, U>::value);
__VEG_CMP(>, cmp_greater, meta::partially_ordered_with<U, T>::value);
__VEG_CMP(<=, cmp_less_equal, meta::partially_ordered_with<U, T>::value);
__VEG_CMP(>=, cmp_greater_equal, meta::partially_ordered_with<T, U>::value);

#undef __VEG_CMP

VEG_TEMPLATE(
    (typename T, typename U),
    requires(meta::equality_comparable_with<T, U>::value),
    HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto
    operator!=,
    (a, option<T> const&),
    (b, option<U> const&))
noexcept -> bool {
  return !option_::cmp(a, b, cmp_equal);
}
VEG_TEMPLATE(
    (typename T),
    requires(true),
    HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto
    operator==,
    (lhs, option<T> const&),
    (/*rhs*/, none_t))
noexcept -> bool {
  return !static_cast<bool>(lhs);
}
VEG_TEMPLATE(
    (typename T),
    requires(true),
    HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto
    operator==,
    (/*lhs*/, none_t),
    (rhs, option<T> const&))
noexcept -> bool {
  return !static_cast<bool>(rhs);
}
VEG_TEMPLATE(
    (typename T),
    requires(true),
    HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto
    operator!=,
    (lhs, option<T> const&),
    (/*rhs*/, none_t))
noexcept -> bool {
  return static_cast<bool>(lhs);
}
VEG_TEMPLATE(
    (typename T),
    requires(true),
    HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto
    operator!=,
    (/*lhs*/, none_t),
    (rhs, option<T> const&))
noexcept -> bool {
  return static_cast<bool>(rhs);
}

template <typename T>
struct option_flatten_base {};

template <typename T>
struct ret_none {
  HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto operator()() const noexcept
      -> option<T> {
    return none;
  }
};

template <typename T>
struct option_flatten_base<option<T>> {
  VEG_NODISCARD HEDLEY_ALWAYS_INLINE
  __VEG_CPP14(constexpr) auto flatten() && noexcept(
      meta::nothrow_constructible<T, T&&>::value) -> option<T> {
    auto&& self = static_cast<option<option<T>>&&>(*this);
    return VEG_FWD(self).as_ref().map_or_else(
        decay_fn<option<T>>{}, ret_none<T>{});
  }
};

template <typename T>
struct cmp_with_fn {
  meta::remove_cvref_t<T> const& rhs;
  HEDLEY_ALWAYS_INLINE constexpr auto
  operator()(meta::remove_cvref_t<T> const& lhs) const noexcept -> bool {
    return static_cast<bool>(rhs == lhs);
  }
};

template <typename T, bool = meta::equality_comparable_with<T, T>::value>
struct eq_cmp_base {
  VEG_NODISCARD HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto contains(
      meta::remove_cvref_t<T> const& val) const noexcept -> bool {
    auto& self = static_cast<option<T> const&>(*this);
    return self.as_cref().map_or(cmp_with_fn<T>{val}, false);
  }
};

template <typename T>
struct eq_cmp_base<T, false> {};

} // namespace option_
} // namespace internal

template <typename T>
struct VEG_NODISCARD option
    : private internal::option_::option_move_ctor_base2<T>,
      public internal::option_::option_copymove_interface_base<T>,
      public internal::option_::option_flatten_base<T>,
      public internal::option_::eq_cmp_base<T> {
  static_assert(meta::move_constructible<T>::value, "err");

  option() = default;
  ~option() = default;
  option(option const&) = default;
  option(option&&) noexcept(meta::nothrow_move_constructible<T>::value) =
      default;
  using internal::option_::option_move_ctor_base2<T>::option_move_ctor_base2;

  auto operator=(option const&) & -> option& = default;
  auto operator=(option&&) & noexcept(
      (meta::nothrow_move_constructible<T>::value &&
       meta::nothrow_move_assignable<T>::value)) -> option& = default;

  __VEG_CPP14(constexpr)
  option // NOLINT(hicpp-explicit-conversions)
      (none_t /*tag*/) noexcept
      : option{} {}

  __VEG_CPP14(constexpr) auto operator=(none_t /*arg*/) noexcept -> option& {
    if (*this) {
      this->destroy();
    }
    return *this;
  }

  VEG_TEMPLATE(
      (typename Fn),
      requires(
          (meta::invocable<Fn&&>::value &&
           __VEG_SAME_AS(T, (meta::detected_t<meta::invoke_result_t, Fn&&>)))),
      __VEG_CPP14(constexpr) void emplace,
      (fn, Fn&&)) &

      noexcept(meta::nothrow_invocable<Fn&&>::value) {
    if (*this) {
      *this = none;
    }
    this->_emplace(VEG_FWD(fn));
  }

  VEG_TEMPLATE(
      (typename Fn),
      requires(meta::invocable<Fn&&, T&&>::value),
      VEG_NODISCARD __VEG_CPP14(constexpr) auto map,
      (fn, Fn&&)) &&

      noexcept(meta::nothrow_invocable<Fn&&, T&&>::value)
          -> option<meta::detected_t<meta::invoke_result_t, Fn&&, T&&>> {

    if (*this) {
      return option<meta::invoke_result_t<Fn&&, T&&>>{
          some,
          inplace,
          VEG_FWD(fn),
          static_cast<option&&>(*this).as_ref().unwrap_unchecked(unsafe)};
    }
    return none;
  }

  VEG_TEMPLATE(
      (typename Fn),
      requires(meta::is_option<
               meta::detected_t<meta::invoke_result_t, Fn&&, T&&>>::value),
      VEG_NODISCARD __VEG_CPP14(constexpr) auto and_then,
      (fn, Fn&&)) &&

      noexcept(meta::nothrow_invocable<Fn&&, T&&>::value)
          -> meta::detected_t<meta::invoke_result_t, Fn&&, T&&> {
    if (*this) {
      return invoke(
          VEG_FWD(fn),
          static_cast<option&&>(*this).as_ref().unwrap_unchecked(unsafe));
    }
    return none;
  }

  VEG_TEMPLATE(
      (typename Fn, typename D),
      requires(
          (meta::invocable<D&&>::value && //
           __VEG_SAME_AS(
               (meta::detected_t<meta::invoke_result_t, Fn&&, T&&>),
               (meta::detected_t<meta::invoke_result_t, D&&>)))),
      VEG_NODISCARD __VEG_CPP14(constexpr) auto map_or_else,
      (fn, Fn&&),
      (d, D&&)) &&

      noexcept(meta::nothrow_invocable<Fn&&>::value)
          -> meta::detected_t<meta::invoke_result_t, Fn&&, T&&> {
    if (*this) {
      return invoke(
          VEG_FWD(fn),
          static_cast<option&&>(*this).as_ref().unwrap_unchecked(unsafe));
    }
    return invoke(VEG_FWD(d));
  }

  VEG_TEMPLATE(
      (typename Fn, typename D),
      requires(
          (meta::move_constructible<
               meta::detected_t<meta::invoke_result_t, Fn&&, T&&>>::value &&
           __VEG_SAME_AS(
               (meta::detected_t<meta::invoke_result_t, Fn&&, T&&> &&), D&&))),
      VEG_NODISCARD __VEG_CPP14(constexpr) auto map_or,
      (fn, Fn&&),
      (d, D&&)) &&

      noexcept(
          (meta::nothrow_move_constructible<
               meta::invoke_result_t<Fn&&, T&&>>::value &&
           meta::nothrow_invocable<Fn&&, T&&>::value))
          -> meta::detected_t<meta::invoke_result_t, Fn&&, T&&> {
    if (!*this) {
      return VEG_FWD(d);
    }
    return invoke(
        VEG_FWD(fn),
        static_cast<option&&>(*this).as_ref().unwrap_unchecked(unsafe));
  }

  VEG_TEMPLATE(
      (typename Fn),
      requires(__VEG_SAME_AS(
          (meta::detected_t<meta::invoke_result_t, Fn&&>), veg::option<T>)),
      VEG_NODISCARD __VEG_CPP14(constexpr) auto or_else,
      (fn, Fn&&)) &&

      noexcept(
          (meta::nothrow_invocable<Fn&&>::value &&
           meta::nothrow_constructible<T, T const&>::value)) -> option<T> {
    if (*this) {
      return {
          some, static_cast<option&&>(*this).as_ref().unwrap_unchecked(unsafe)};
    }
    return invoke(VEG_FWD(fn));
  }

  VEG_NODISCARD HEDLEY_ALWAYS_INLINE
  __VEG_CPP14(constexpr) auto as_cref() const noexcept
      -> option<meta::remove_cvref_t<T> const&> {
    if (*this) {
      return {some, as_ref().unwrap_unchecked(unsafe)};
    }
    return none;
  }

  VEG_NODISCARD HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) explicit
  operator bool() const noexcept {
    return this->is_engaged();
  }

  __VEG_CVREF_DUPLICATE(
      VEG_NODISCARD HEDLEY_ALWAYS_INLINE __VEG_CPP14(constexpr) auto as_ref(),
      internal::option_::as_ref_impl<T>,
      ());

private:
  template <typename U, bool B>
  friend struct internal::option_::value_sentinel_impl;

  friend struct internal::option_::option_copy_interface_base<T>;
  friend struct internal::option_::option_move_interface_base<T>;

  template <typename U, typename Self>
  friend __VEG_CPP14(constexpr) auto internal::option_::as_ref_impl(
      Self&& self) noexcept -> option<meta::collapse_category_t<U, Self&&>>;
};
__VEG_CPP17(

    template <typename T> option(some_t, T) -> option<T>;
    template <typename T> option(some_ref_t, T&&) -> option<T&&>;

)

template <typename T>
struct meta::value_sentinel_for<option<T>>
    : veg::internal::option_::value_sentinel_impl<T> {};

} // namespace veg

#endif /* end of include guard __VEG_OPTION_HPP_8NVLXES2S */
