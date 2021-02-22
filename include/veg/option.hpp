#ifndef VEG_OPTION_HPP_8NVLXES2S
#define VEG_OPTION_HPP_8NVLXES2S

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
VEG_ODR_VAR(none, none_t);
struct some_t {
  VEG_TEMPLATE(
      (typename T),
      requires(meta::constructible<meta::remove_cvref_t<T>, T&&>::value),
      VEG_CPP14(constexpr) auto
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
VEG_ODR_VAR(some, some_t);
struct some_ref_t {
  template <typename T>
  VEG_CPP14(constexpr)
  auto operator()(T&& arg) const noexcept -> option<T&&> {
    return {some, VEG_FWD(arg)};
  }

private:
  constexpr some_ref_t() = default;
  constexpr explicit some_ref_t(some_ref_t* /*unused*/) noexcept {}
  template <typename T>
  friend struct meta::internal::static_const;
};
VEG_ODR_VAR(some_ref, some_ref_t);

namespace internal {
namespace option_ {

enum kind { has_sentinel, trivial, non_trivial };

template <typename T, bool = (meta::value_sentinel_for<T>::value > 1)>
struct value_sentinel_impl
    : std::integral_constant<i64, i64(static_cast<unsigned char>(-3))> {
  static VEG_CPP14(constexpr) auto invalid(i64 i) noexcept -> option<T> {
    if (i <= static_cast<unsigned char>(-3)) {
      option<T> val = none;
      val.engaged = static_cast<unsigned char>(2 + i);
      return val;
    }
    terminate();
  }
  static VEG_CPP14(constexpr) auto id(option<T> const& arg) noexcept -> i64 {
    return (arg.engaged < 2) ? -1 : (arg.engaged - 2);
  }
};

template <typename T>
struct value_sentinel_impl<T, true>
    : std::integral_constant<i64, meta::value_sentinel_for<T>::value - 1> {
  static VEG_CPP14(constexpr) auto invalid(i64 i) noexcept -> option<T> {
    return option<T>{meta::value_sentinel_for<T>::invalid(i + 1)};
  }
  static VEG_CPP14(constexpr) auto id(option<T> const& arg) noexcept -> i64 {
    if (arg) {
      return -1;
    }
    return meta::value_sentinel_for<T>::id(
        arg.unwrap_unchecked_invalid(unsafe));
  }
};

template <typename T, typename Self>
VEG_CPP14(constexpr)
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
  VEG_CPP20(constexpr) void operator()() const noexcept {
    mem::construct_at(c);
  }
};

template <typename T>
using kind_v = std::integral_constant<
    kind,
    ((meta::value_sentinel_for<storage::storage<T>>::value > 0) //
         ? has_sentinel
         : ((meta::mostly_trivial<T>::value && meta::constructible<T>::value))
               ? trivial
               : non_trivial)>;

template <typename T, kind = kind_v<T>::value>
// trivial
struct option_storage_base {
  storage::storage<T> some = {};
  unsigned char engaged = 0;

  VEG_CPP14(constexpr) option_storage_base() = default;
  VEG_CPP14(constexpr)
  explicit option_storage_base(T&& val) noexcept(
      meta::nothrow_move_constructible<T>::value)
      : some(VEG_FWD(val)), engaged(1) {}

  VEG_NODISCARD VEG_CPP14(constexpr) auto is_engaged() const noexcept -> bool {
    return engaged == 1;
  }

  template <typename Fn>
  VEG_CPP14(constexpr)
  void _emplace(Fn&& fn) noexcept(meta::nothrow_invocable<Fn&&>::value) {
    VEG_DEBUG_ASSERT(!engaged);
    some = storage::storage<T>(veg::invoke(VEG_FWD(fn)));
    engaged = 1;
  }

  template <typename U>
  VEG_CPP14(constexpr)
  void assign(U&& rhs) noexcept(noexcept(some.assign(VEG_FWD(rhs)))) {
    VEG_DEBUG_ASSERT(engaged);
    some.assign(VEG_FWD(rhs));
  }

  VEG_CPP14(constexpr) void destroy() noexcept {
    VEG_DEBUG_ASSERT(engaged);
    engaged = 0;
  }

  VEG_NODISCARD VEG_CPP14(constexpr) auto _get() const& noexcept -> T const& {
    VEG_DEBUG_ASSERT(is_engaged());
    return some._get();
  }
  VEG_NODISCARD VEG_CPP14(constexpr) auto _get() & noexcept -> T& {
    VEG_DEBUG_ASSERT(is_engaged());
    return some.get_mut();
  }
  VEG_NODISCARD VEG_CPP14(constexpr) auto _get() && noexcept -> T&& {
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

  VEG_CPP14(constexpr) option_storage_base() = default;
  VEG_CPP14(constexpr)
  explicit option_storage_base(T&& val) noexcept(
      meta::nothrow_move_constructible<T>::value)
      : some(VEG_FWD(val)) {}

  VEG_NODISCARD VEG_CPP14(constexpr) auto is_engaged() const noexcept -> bool {
    return sentinel_traits::id(some) < 0;
  }

  template <typename Fn>
  VEG_CPP14(constexpr)
  void _emplace(Fn&& fn) noexcept(meta::nothrow_invocable<Fn&&>::value) {
    VEG_DEBUG_ASSERT(!is_engaged());
    some = storage::storage<T>(veg::invoke(VEG_FWD(fn)));
  }

  template <typename U>
  VEG_CPP14(constexpr)
  void assign(U&& rhs) noexcept(noexcept(some.assign(VEG_FWD(rhs)))) {
    VEG_DEBUG_ASSERT(is_engaged());
    some.assign(VEG_FWD(rhs));
  }

  VEG_CPP14(constexpr) void destroy() noexcept {
    some = sentinel_traits::invalid(0);
  }

  VEG_NODISCARD VEG_CPP14(constexpr) auto unwrap_unchecked_invalid(
      unsafe_t /*tag*/) const& noexcept -> T const& {
    return some._get();
  }

  VEG_NODISCARD VEG_CPP14(constexpr) auto _get() const& noexcept -> T const& {
    VEG_DEBUG_ASSERT(is_engaged());
    return some._get();
  }
  VEG_NODISCARD VEG_CPP14(constexpr) auto _get() & noexcept -> T& {
    VEG_DEBUG_ASSERT(is_engaged());
    return some.get_mut();
  }
  VEG_NODISCARD VEG_CPP14(constexpr) auto _get() && noexcept -> T&& {
    VEG_DEBUG_ASSERT(is_engaged());
    return VEG_FWD(some).get_mov_ref();
  }
};

template <typename T, bool = meta::mostly_trivial<T>::value>
struct option_storage_base_option_emplace {
  template <typename Fn>
  VEG_CPP14(constexpr)
  void _emplace(Fn&& fn) noexcept(meta::nothrow_invocable<Fn&&>::value) {
    auto& self = *static_cast<option_storage_base<option<T>>*>(this);
    VEG_DEBUG_ASSERT(!self.is_engaged());
    self.some = VEG_FWD(fn)();
  }
};

template <typename T>
struct option_storage_base_option_emplace<T, false> {
  template <typename Fn>
  VEG_CPP14(constexpr)
  void _emplace(Fn&& fn) noexcept(meta::nothrow_invocable<Fn&&>::value) {
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

  VEG_CPP14(constexpr) option_storage_base() = default;
  VEG_CPP14(constexpr)
  explicit option_storage_base(option<T>&& val) noexcept(
      meta::nothrow_move_constructible<T>::value)
      : some(VEG_FWD(val)) {}

  VEG_NODISCARD VEG_CPP14(constexpr) auto is_engaged() const noexcept -> bool {
    return sentinel_traits::id(some) < 0;
  }

  template <typename U>
  VEG_CPP14(constexpr)
  void assign(U&& rhs) noexcept(noexcept(some.assign(VEG_FWD(rhs)))) {
    VEG_DEBUG_ASSERT(is_engaged());
    some = VEG_FWD(rhs);
  }

  VEG_CPP14(constexpr) void destroy() noexcept {
    some = sentinel_traits::invalid(0);
  }

  VEG_NODISCARD VEG_CPP14(constexpr) auto unwrap_unchecked_invalid(
      unsafe_t /*tag*/) const& noexcept -> option<T> const& {
    return some;
  }

  VEG_NODISCARD VEG_CPP14(constexpr) auto _get() const& noexcept
      -> option<T> const& {
    VEG_DEBUG_ASSERT(is_engaged());
    return some;
  }
  VEG_NODISCARD VEG_CPP14(constexpr) auto _get() & noexcept -> option<T>& {
    VEG_DEBUG_ASSERT(is_engaged());
    return some;
  }
  VEG_NODISCARD VEG_CPP14(constexpr) auto _get() && noexcept -> option<T>&& {
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

  VEG_CPP14(constexpr) option_storage_nontrivial_base() noexcept {};
  VEG_CPP14(constexpr)
  option_storage_nontrivial_base(i64 /* unused */, unsigned char id) noexcept
      : engaged{id} {};
  VEG_CPP14(constexpr)
  explicit option_storage_nontrivial_base(T&& val) //
      noexcept(meta::nothrow_move_constructible<T>::value)
      : some(VEG_FWD(val)), engaged(1) {}

  VEG_NODISCARD VEG_CPP14(constexpr) auto is_engaged() const noexcept -> bool {
    return engaged == 1;
  }
  VEG_CPP14(constexpr) void set_engaged(bool b) noexcept {
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
  VEG_CPP14(constexpr) option_storage_nontrivial_base() noexcept {};
  VEG_CPP14(constexpr)
  option_storage_nontrivial_base(i64 /* unused */, unsigned char id) noexcept
      : engaged{id} {};
  VEG_CPP14(constexpr)
  explicit option_storage_nontrivial_base(T&& val) //
      noexcept(meta::nothrow_move_constructible<T>::value)
      : some(VEG_FWD(val)), engaged(1) {}

  VEG_NODISCARD VEG_CPP14(constexpr) auto is_engaged() const noexcept -> bool {
    return engaged == 1;
  }
  VEG_CPP14(constexpr) void set_engaged(bool b) noexcept {
    engaged = static_cast<unsigned char>(b);
  }

  VEG_CPP20(constexpr) ~option_storage_nontrivial_base() {
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
  VEG_CPP14(constexpr)
  void assign(U&& rhs) noexcept(noexcept(some.assign(VEG_FWD(rhs)))) {
    VEG_DEBUG_ASSERT(is_engaged());
    some.assign(VEG_FWD(rhs));
  }

  template <typename Fn>
  VEG_CPP20(constexpr)
  void _emplace(Fn&& fn) noexcept(meta::nothrow_invocable<Fn&&>::value) {
    VEG_DEBUG_ASSERT(!is_engaged());

    finally<do_if<make_none>> guard{{true, {mem::addressof(none)}}};

    mem::construct_at(mem::addressof(some), 0, VEG_FWD(fn));

    guard.fn.cond = false;
    set_engaged(true);
  }

  VEG_CPP20(constexpr) void destroy() noexcept {
    VEG_DEBUG_ASSERT(is_engaged());
    set_engaged(false);
    finally<make_none> guard{mem::addressof(none)};
    mem::destroy_at(mem::addressof(some));
  }

  VEG_CPP14(constexpr) auto _get() const& noexcept -> T const& {
    VEG_DEBUG_ASSERT(is_engaged());
    return some._get();
  }
  VEG_CPP14(constexpr) auto _get() & noexcept -> T& {
    VEG_DEBUG_ASSERT(is_engaged());
    return some.get_mut();
  }
  VEG_CPP14(constexpr) auto _get() && noexcept -> T&& {
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
  VEG_CPP14(constexpr)
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
  VEG_CPP14(constexpr)
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
  VEG_CPP14(constexpr)
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
  VEG_CPP14(constexpr)
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

template <typename T, bool = meta::constructible<T, T const&>::value>
struct option_nocopy_interface_base {

  VEG_CPP14(constexpr)
  auto clone() && noexcept(meta::nothrow_move_constructible<T>::value)
      -> option<T> {
    auto&& self = static_cast<option<T>&&>(*this);
    if (self) {
      return {some, VEG_FWD(self).as_ref().unwrap_unchecked(unsafe)};
    }
    return none;
  }
};

template <typename T>
struct option_nocopy_interface_base<T, true>
    : option_nocopy_interface_base<T, false> {

  using option_nocopy_interface_base<T, false>::clone;

  VEG_CPP14(constexpr)
  auto clone() const& noexcept(meta::nothrow_constructible<T, T const&>::value)
      -> option<T> {
    auto const& self = static_cast<option<T> const&>(*this);
    if (self) {
      return {some, self.as_ref().unwrap_unchecked(unsafe)};
    }
    return none;
  }
};

template <typename T, typename U, typename Fn>
VEG_CPP14(constexpr)
auto cmp(option<T> const& lhs, option<U> const& rhs, Fn fn) noexcept -> bool {
  if (lhs) {
    if (rhs) {
      return static_cast<bool>(
          fn(lhs.as_cref().unwrap_unchecked(unsafe),
             rhs.as_cref().unwrap_unchecked(unsafe)));
    }
  }
  return (static_cast<bool>(lhs) == static_cast<bool>(rhs));
}

#define VEG_CMP(op, fn, ...)                                                   \
  VEG_TEMPLATE(                                                                \
      (typename T, typename U),                                                \
      requires(__VA_ARGS__),                                                   \
      VEG_NODISCARD VEG_CPP14(constexpr) auto                                  \
      operator op,                                                             \
      (lhs, option<T> const&),                                                 \
      (rhs, option<U> const&))                                                 \
  noexcept->bool {                                                             \
    return option_::cmp(lhs, rhs, fn);                                         \
  }                                                                            \
  static_assert(true, "")

VEG_CMP(==, cmp_equal, meta::is_equality_comparable_with<T, U>::value);
VEG_CMP(<, cmp_less, meta::is_partially_ordered_with<T, U>::value);
VEG_CMP(>, cmp_greater, meta::is_partially_ordered_with<U, T>::value);
VEG_CMP(<=, cmp_less_equal, meta::is_partially_ordered_with<U, T>::value);
VEG_CMP(>=, cmp_greater_equal, meta::is_partially_ordered_with<T, U>::value);

#undef VEG_CMP

VEG_TEMPLATE(
    (typename T, typename U),
    requires(meta::is_equality_comparable_with<T, U>::value),
    VEG_CPP14(constexpr) auto
    operator!=,
    (a, option<T> const&),
    (b, option<U> const&))
noexcept(noexcept(option_::cmp(a, b, cmp_equal))) -> bool {
  return !option_::cmp(a, b, cmp_equal);
}
VEG_TEMPLATE(
    (typename T),
    requires(true),
    VEG_CPP14(constexpr) auto
    operator==,
    (lhs, option<T> const&),
    (/*rhs*/, none_t))
noexcept -> bool {
  return !static_cast<bool>(lhs);
}
VEG_TEMPLATE(
    (typename T),
    requires(true),
    VEG_CPP14(constexpr) auto
    operator==,
    (/*lhs*/, none_t),
    (rhs, option<T> const&))
noexcept -> bool {
  return !static_cast<bool>(rhs);
}
VEG_TEMPLATE(
    (typename T),
    requires(true),
    VEG_CPP14(constexpr) auto
    operator!=,
    (lhs, option<T> const&),
    (/*rhs*/, none_t))
noexcept -> bool {
  return static_cast<bool>(lhs);
}
VEG_TEMPLATE(
    (typename T),
    requires(true),
    VEG_CPP14(constexpr) auto
    operator!=,
    (/*lhs*/, none_t),
    (rhs, option<T> const&))
noexcept -> bool {
  return static_cast<bool>(rhs);
}

template <typename T>
struct option_flatten_base {};

template <typename T>
struct option_flatten_base<option<T>> {
  VEG_NODISCARD VEG_CPP14(constexpr) auto flatten() && noexcept(
      meta::nothrow_constructible<T, T&&>::value) -> option<T> {
    if (static_cast<option<option<T>> const&>(*this)) {
      return static_cast<option<option<T>>&&>(*this).as_ref().unwrap_unchecked(
          unsafe);
    }
    return none;
  }
};

template <typename T, bool = meta::is_equality_comparable_with<T, T>::value>
struct eq_cmp_base {
  VEG_NODISCARD
  VEG_CPP14(constexpr)
  auto contains(T const& val) const noexcept -> bool {
    auto& self = static_cast<option<T> const&>(*this);
    if (self) {
      return self.as_ref().unwrap_unchecked(unsafe) == val;
    }
    return false;
  }
};

template <typename T>
struct eq_cmp_base<T, false> {};

} // namespace option_
} // namespace internal

template <typename T>
struct VEG_NODISCARD option
    : private internal::option_::option_move_assign_base<T>,
      public internal::option_::option_nocopy_interface_base<T>,
      public internal::option_::option_flatten_base<T>,
      public internal::option_::eq_cmp_base<T> {
  static_assert(meta::move_constructible<T>::value, "err");

  option() = default;
  ~option() = default;
  option(option const&) = default;
  option(option&&) noexcept(meta::nothrow_move_constructible<T>::value) =
      default;
  auto operator=(option const&) & -> option& = default;
  auto operator=(option&&) & noexcept(
      (meta::nothrow_move_constructible<T>::value &&
       meta::nothrow_move_assignable<T>::value)) -> option& = default;

  explicit VEG_CPP14(constexpr)
      option(T value) noexcept(meta::nothrow_constructible<T, T&&>::value)
      : internal::option_::option_move_assign_base<T>(VEG_FWD(value)) {}

  VEG_CPP14(constexpr)
  option(some_t /*tag*/, T value) noexcept(
      meta::nothrow_constructible<T, T&&>::value)
      : internal::option_::option_move_assign_base<T>(VEG_FWD(value)) {}
  VEG_CPP14(constexpr)
  option // NOLINT(hicpp-explicit-conversions)
      (none_t /*tag*/) noexcept
      : option{} {}
  VEG_CPP14(constexpr) auto operator=(none_t /*arg*/) noexcept -> option& {
    if (*this) {
      this->destroy();
    }
    return *this;
  }

  VEG_TEMPLATE(
      (typename Fn),
      requires(
          (meta::invocable<Fn&&>::value &&
           VEG_SAME_AS(T, (meta::detected_t<meta::invoke_result_t, Fn&&>)))),
      VEG_CPP14(constexpr) void emplace,
      (fn, Fn&&))
  &noexcept(meta::nothrow_invocable<Fn&&>::value) {
    if (*this) {
      *this = none;
    }
    this->_emplace(VEG_FWD(fn));
  }

  VEG_TEMPLATE(
      (typename Fn),
      requires(meta::invocable<Fn&&, T&&>::value),
      VEG_NODISCARD VEG_CPP14(constexpr) auto map,
      (fn, Fn&&)) && noexcept(meta::nothrow_invocable<Fn&&, T&&>::value)
      -> option<meta::detected_t<meta::invoke_result_t, Fn&&, T&&>> {

    if (!*this) {
      return none;
    }
    return option<meta::invoke_result_t<Fn&&, T&&>>{invoke(
        VEG_FWD(fn),
        static_cast<option&&>(*this).as_ref().unwrap_unchecked(unsafe))};
  }

  VEG_TEMPLATE(
      (typename Fn),
      requires(meta::is_option<
               meta::detected_t<meta::invoke_result_t, Fn&&, T&&>>::value),
      VEG_NODISCARD VEG_CPP14(constexpr) auto and_then,
      (fn, Fn&&)) && noexcept(meta::nothrow_invocable<Fn&&, T&&>::value)
      -> meta::detected_t<meta::invoke_result_t, Fn&&, T&&> {
    if (!*this) {
      return none;
    }
    return invoke(
        VEG_FWD(fn),
        static_cast<option&&>(*this).as_ref().unwrap_unchecked(unsafe));
  }

  VEG_TEMPLATE(
      (typename Fn, typename D),
      requires(
          (meta::invocable<D&&>::value && //
           VEG_SAME_AS(
               (meta::detected_t<meta::invoke_result_t, Fn&&, T&&>),
               (meta::detected_t<meta::invoke_result_t, D&&>)))),
      VEG_NODISCARD VEG_CPP14(constexpr) auto map_or_else,
      (fn, Fn&&),
      (d, D&&)) && noexcept(meta::nothrow_invocable<Fn&&>::value)
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
      requires(VEG_SAME_AS(
          (meta::detected_t<meta::invoke_result_t, Fn&&, T&&> &&), D&&)),
      VEG_NODISCARD VEG_CPP14(constexpr) auto map_or,
      (fn, Fn&&),
      (d, D&&)) && noexcept(meta::nothrow_invocable<Fn&&, T&&>::value)
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
      requires(VEG_SAME_AS(
          (meta::detected_t<meta::invoke_result_t, Fn&&>), veg::option<T>)),
      VEG_NODISCARD VEG_CPP14(constexpr) auto or_else,
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

  VEG_TEMPLATE(
      (typename Fn),
      requires(meta::constructible<
               bool,
               meta::detected_t<meta::invoke_result_t, Fn, T const&>>::value&&
                   meta::move_constructible<T>::value),
      VEG_NODISCARD VEG_CPP14(constexpr) auto filter,
      (fn, Fn&&))

  &&noexcept((meta::nothrow_invocable<Fn&&, T const&>::value &&
              meta::nothrow_move_constructible<T>::value))
        ->option<T> {
    if (*this) {
      if (invoke(VEG_FWD(fn), as_cref().unwrap_unchecked(unsafe))) {
        return {
            some,
            static_cast<option&&>(*this).as_ref().unwrap_unchecked(unsafe)};
      }
    }
    return none;
  }

  VEG_NODISCARD VEG_CPP14(constexpr) auto as_cref() const noexcept
      -> option<meta::remove_cvref_t<T> const&> {
    if (*this) {
      return {some, as_ref().unwrap_unchecked(unsafe)};
    }
    return none;
  }

  VEG_NODISCARD VEG_CPP14(constexpr) explicit operator bool() const noexcept {
    return this->is_engaged();
  }

  VEG_NODISCARD
  VEG_CPP14(constexpr)
  auto take() noexcept(meta::move_constructible<T>::value) -> option<T> {
    if (*this) {
      T val = static_cast<option&&>(*this)._get();
      *this = none;
      return {some, VEG_FWD(val)};
    }
    return none;
  }

  VEG_CVREF_DUPLICATE(
      VEG_NODISCARD VEG_CPP14(constexpr) auto as_ref(),
      internal::option_::as_ref_impl<T>,
      ());

  VEG_NODISCARD HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto unwrap_unchecked(
      unsafe_t /*tag*/) && noexcept(meta::nothrow_move_constructible<T>::value)
      -> T {
    return static_cast<option&&>(*this)._get();
  }

  VEG_NODISCARD VEG_CPP14(constexpr) auto unwrap() && noexcept(
      meta::nothrow_move_constructible<T>::value) -> T {
    VEG_ASSERT(*this);
    return static_cast<option&&>(*this).unwrap_unchecked(unsafe);
  }

private:
  template <typename U, bool B>
  friend struct internal::option_::value_sentinel_impl;

  template <typename U, bool B>
  friend struct internal::option_::option_nocopy_interface_base;

  template <typename U, typename Self>
  friend VEG_CPP14(constexpr) auto internal::option_::as_ref_impl(
      Self&& self) noexcept -> option<meta::collapse_category_t<U, Self&&>>;
};
VEG_CPP17(

    template <typename T> option(some_t, T) -> option<T>;
    template <typename T> option(some_ref_t, T&&) -> option<T&&>;

)

template <typename T>
struct meta::value_sentinel_for<option<T>>
    : veg::internal::option_::value_sentinel_impl<T> {};

} // namespace veg

#endif /* end of include guard VEG_OPTION_HPP_8NVLXES2S */
