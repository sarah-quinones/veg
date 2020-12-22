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
private:
  constexpr none_t() = default;
  constexpr explicit none_t(none_t* /*unused*/) {}
  template <typename T>
  friend struct meta::internal::static_const;
};
VEG_ODR_VAR(none, none_t);
struct some_t {
private:
  constexpr some_t() = default;
  constexpr explicit some_t(some_t* /*unused*/) {}
  template <typename T>
  friend struct meta::internal::static_const;
};
VEG_ODR_VAR(some, some_t);

namespace internal {
namespace option_ {

enum kind { has_sentinel, trivial, non_trivial };

template <
    typename T,
    kind = (meta::value_sentinel_for<T>::value > 1) ? has_sentinel
           : meta::is_trivially_copyable<T>::value  ? trivial
                                                    : non_trivial>
struct value_sentinel_impl;

template <typename T>
struct value_sentinel_impl<T, has_sentinel>
    : std::integral_constant<i64, meta::value_sentinel_for<T>::value - 1> {
  static constexpr auto invalid(i64 i) {
    return option<T>{meta::value_sentinel_for<T>::invalid(i + 1)};
  }
  static constexpr auto id(option<T> const& arg) {
    return meta::value_sentinel_for<T>::id(arg.as_ref().unwrap());
  }
};
template <typename T>
struct value_sentinel_impl<T, trivial>
    : std::integral_constant<i64, i64(static_cast<unsigned char>(-3))> {
  static constexpr auto invalid(i64 i) {
    if (i <= static_cast<unsigned char>(-3)) {
      option<T> val = none;
      val.engaged = static_cast<unsigned char>(2 + i);
      return val;
    }
    terminate();
  }
  static constexpr auto id(option<T> const& arg) {
    return arg.engaged < 2 ? -1 : arg.engaged - 2;
  }
};

template <typename T, typename Self>
constexpr auto as_ref_impl(Self&& self) noexcept
    -> option<meta::collapse_category_t<T, Self&&>> {
  if (self) {
    return {some, VEG_FWD(self).get()};
  }
  return none;
}

template <typename Fn>
struct finally { // NOLINT(cppcoreguidelines-special-member-functions)
  Fn fn;
  VEG_CPP20(constexpr) ~finally() { fn(); }
};

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

template <
    typename T,
    kind = (meta::value_sentinel_for<storage::storage<T>>::value > 0) //
               ? has_sentinel
               : ((meta::is_mostly_trivial<T>::value &&
                   meta::is_default_constructible<T>::value))
                     ? trivial
                     : non_trivial>
// trivial
struct option_storage_base {
  storage::storage<T> some = {};
  unsigned char engaged = 0;

  constexpr option_storage_base() = default;
  constexpr explicit option_storage_base(std::remove_const_t<T>&& val)
      : some(VEG_FWD(val)), engaged(1) {}

  VEG_NODISCARD constexpr auto is_engaged() const noexcept -> bool {
    return engaged == 1;
  }

  template <typename... Args>
  constexpr auto emplace(Args&&... args) noexcept(
      meta::is_nothrow_constructible<T, Args&&...>::value) {
    VEG_DEBUG_ASSERT(!engaged);
    some = storage::storage<T>(T(args...));
    engaged = true;
  }

  template <typename U>
  constexpr auto assign(U&& rhs) noexcept(noexcept(some.assign(VEG_FWD(rhs)))) {
    VEG_DEBUG_ASSERT(engaged);
    some.assign(VEG_FWD(rhs));
  }

  constexpr auto destroy() noexcept {
    VEG_DEBUG_ASSERT(engaged);
    engaged = 0;
  }

  VEG_NODISCARD constexpr auto get() const& noexcept -> T const& {
    VEG_DEBUG_ASSERT(is_engaged());
    return some.get();
  }
  VEG_NODISCARD constexpr auto get() & noexcept -> std::remove_const_t<T>& {
    VEG_DEBUG_ASSERT(is_engaged());
    return some.get_mut();
  }
  VEG_NODISCARD constexpr auto get() && noexcept -> std::remove_const_t<T>&& {
    VEG_DEBUG_ASSERT(is_engaged());
    return VEG_MOV(some).get_mov_ref();
  }
};

template <typename T>
struct option_storage_base<T, has_sentinel> {
  using sentinel_traits = meta::value_sentinel_for<storage::storage<T>>;

  static_assert(meta::is_trivially_destructible<T>::value);
  static_assert(meta::is_trivially_copyable<storage::storage<T>>::value);

  storage::storage<T> some = sentinel_traits::invalid(0);

  constexpr option_storage_base() = default;
  constexpr explicit option_storage_base(std::remove_const_t<T>&& val)
      : some(VEG_FWD(val)) {}

  VEG_NODISCARD constexpr auto is_engaged() const noexcept -> bool {
    return sentinel_traits::id(some) < 0;
  }

  template <typename... Args>
  constexpr auto emplace(Args&&... args) noexcept(
      meta::is_nothrow_constructible<T, Args&&...>::value) {
    VEG_DEBUG_ASSERT(!is_engaged());
    some = storage::storage<T>(T(args...));
  }

  template <typename U>
  constexpr auto assign(U&& rhs) noexcept(noexcept(some.assign(VEG_FWD(rhs)))) {
    VEG_DEBUG_ASSERT(is_engaged());
    some.assign(VEG_FWD(rhs));
  }

  constexpr auto destroy() noexcept { some = sentinel_traits::invalid(0); }

  VEG_NODISCARD constexpr auto get() const& noexcept -> T const& {
    VEG_DEBUG_ASSERT(is_engaged());
    return some.get();
  }
  VEG_NODISCARD constexpr auto get() & noexcept -> std::remove_const_t<T>& {
    VEG_DEBUG_ASSERT(is_engaged());
    return some.get_mut();
  }
  VEG_NODISCARD constexpr auto get() && noexcept -> std::remove_const_t<T>&& {
    VEG_DEBUG_ASSERT(is_engaged());
    return VEG_MOV(some).get_mov_ref();
  }
};

template <typename T, bool = meta::is_trivially_destructible<T>::value>
struct option_storage_nontrivial_base {
  union {
    unsigned char none = {};
    storage::storage<T> some;
  };
  bool engaged = false;

  constexpr option_storage_nontrivial_base() noexcept {};
  constexpr explicit option_storage_nontrivial_base(
      meta::remove_cv_t<T>&& val) //
      noexcept(meta::is_nothrow_move_constructible<meta::remove_cv_t<T>>::value)
      : some(VEG_FWD(val)), engaged(true) {}

  VEG_NODISCARD constexpr auto is_engaged() const noexcept -> bool {
    return engaged;
  }
  constexpr void set_engaged(bool b) noexcept { engaged = b; }
};

template <typename T>
struct
    option_storage_nontrivial_base // NOLINT(cppcoreguidelines-special-member-functions)
    <T, false> {
  union {
    unsigned char none = {};
    storage::storage<T> some;
  };
  bool engaged = false;
  constexpr option_storage_nontrivial_base() noexcept {};
  constexpr explicit option_storage_nontrivial_base(
      meta::remove_cv_t<T>&& val) //
      noexcept(meta::is_nothrow_move_constructible<meta::remove_cv_t<T>>::value)
      : some(VEG_FWD(val)), engaged(true) {}

  VEG_NODISCARD constexpr auto is_engaged() const noexcept -> bool {
    return engaged;
  }
  constexpr void set_engaged(bool b) noexcept { engaged = b; }

  VEG_CPP20(constexpr) ~option_storage_nontrivial_base() {
    if (engaged) {
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
  constexpr auto assign(U&& rhs) noexcept(noexcept(some.assign(VEG_FWD(rhs)))) {
    VEG_DEBUG_ASSERT(is_engaged());
    some.assign(VEG_FWD(rhs));
  }

  template <typename... Args>
  VEG_CPP20(constexpr)
  auto emplace(Args&&... args) noexcept(
      meta::is_nothrow_constructible<T, Args&&...>::value) {
    VEG_DEBUG_ASSERT(!is_engaged());

    finally<do_if<make_none>> guard{{true, {mem::addressof(none)}}};

    mem::construct_at(mem::addressof(some), VEG_FWD(args)...);

    guard.fn.cond = false;
    set_engaged();
  }

  VEG_CPP20(constexpr) auto destroy() noexcept {
    VEG_DEBUG_ASSERT(is_engaged());
    set_engaged(false);
    finally<make_none> guard{mem::addressof(none)};
    mem::destroy_at(mem::addressof(some));
  }

  constexpr auto get() const& noexcept -> T const& {
    VEG_DEBUG_ASSERT(is_engaged());
    return some.get();
  }
  constexpr auto get() & noexcept -> std::remove_const_t<T>& {
    VEG_DEBUG_ASSERT(is_engaged());
    return some.get_mut();
  }
  constexpr auto get() && noexcept -> std::remove_const_t<T>&& {
    VEG_DEBUG_ASSERT(is_engaged());
    return VEG_MOV(some).get_mov_ref();
  }
};

template <
    typename T,
    bool =
        meta::is_reference<T>::value || meta::is_trivially_copyable<T>::value>
struct option_copy_base : option_storage_base<T> {
  using option_storage_base<T>::option_storage_base;
};

template <typename T>
struct option_copy_base<T, false> : option_storage_base<T> {
  using option_storage_base<T>::option_storage_base;
  ~option_copy_base() = default;

  constexpr option_copy_base(option_copy_base const& rhs) noexcept(
      meta::is_nothrow_constructible<T, T const&>::value)
      : option_copy_base{} {
    if (rhs.is_engaged()) {
      this->emplace(rhs.get());
    }
  }
  constexpr option_copy_base(option_copy_base&& rhs) noexcept(
      meta::is_nothrow_constructible<T, T&&>::value)
      : option_copy_base{} {
    if (rhs.is_engaged()) {
      this->emplace(VEG_MOV(rhs).get());
    }
  }

  constexpr auto
  operator= /* NOLINT(cert-oop54-cpp) */(option_copy_base const& rhs) noexcept(
      (meta::is_nothrow_constructible<T, T const&>::value &&
       meta::is_nothrow_assignable<T&, T const&>::value)) -> option_copy_base& {
    if (rhs.is_engaged()) {
      if (this->is_engaged()) {
        this->assign(rhs.get());
      } else {
        this->emplace(rhs.get());
      }
    } else {
      if (this->is_engaged()) {
        this->destroy();
      }
    }
    return *this;
  }
  constexpr auto operator=(option_copy_base&& rhs) noexcept(
      (meta::is_nothrow_constructible<T, T&&>::value &&
       meta::is_nothrow_assignable<T&, T&&>::value)) -> option_copy_base& {
    if (rhs.is_engaged()) {
      if (this->is_engaged()) {
        this->assign(VEG_MOV(rhs).get());
      } else {
        this->emplace(VEG_MOV(rhs).get());
      }
    } else {
      if (this->is_engaged()) {
        this->destroy();
      }
    }
    return *this;
  }
};

template <typename T, typename Ret, typename Self, typename Fn>
constexpr auto and_then_impl(Self&& self, Fn&& fn) noexcept(
    meta::is_nothrow_invocable<Fn&&, meta::collapse_category_t<T, Self&&>>::
        value) -> Ret {
  if (!self) {
    return none;
  }
  return invoke(VEG_FWD(fn), VEG_FWD(self).as_ref().unwrap());
}

template <typename T, typename Ret, typename Self, typename Fn>
constexpr auto map_impl(Self&& self, Fn&& fn) noexcept(
    meta::is_nothrow_invocable<Fn&&, meta::collapse_category_t<T, Self&&>>::
        value) -> option<Ret> {

  if (!self) {
    return none;
  }
  return option<Ret>{invoke(VEG_FWD(fn), VEG_FWD(self).as_ref().unwrap())};
}

template <typename T, typename Ret, typename Self, typename D, typename Fn>
constexpr auto map_or_impl(Self&& self, Fn&& fn, D&& d) noexcept(
    (meta::is_nothrow_invocable<Fn&&, meta::collapse_category_t<T, Self&&>>::
         value &&
     meta::is_nothrow_constructible<Ret, D&&>::value)) -> Ret {

  if (!self) {
    return static_cast<Ret>(VEG_FWD(d));
  }
  return invoke(VEG_FWD(fn), VEG_FWD(self).as_ref().unwrap());
}

template <typename T, typename Self, typename Fn>
constexpr auto or_else_impl(Self&& self, Fn&& fn) noexcept(
    meta::is_nothrow_invocable<Fn&&>::value) -> option<T> {
  if (self) {
    return {some, VEG_FWD(self).unwrap()};
  }
  return invoke(VEG_FWD(fn));
}

template <typename T, typename Ret, typename Self, typename D, typename Fn>
constexpr auto map_or_else_impl(Self&& self, Fn&& fn, D&& d) noexcept(
    meta::is_nothrow_invocable<Fn&&>::value) -> Ret {
  if (self) {
    return invoke(VEG_FWD(fn), VEG_FWD(self).as_ref().unwrap());
  }
  return invoke(VEG_FWD(d));
}

template <typename T, typename Self>
constexpr auto unwrap_impl(Self&& self) noexcept
    -> meta::collapse_category_t<T, Self> {
  VEG_ASSERT(self);
  return {VEG_FWD(self).get()};
}

template <typename T, bool = meta::is_copy_constructible<T>::value>
struct option_move_only_methods_base : protected option_copy_base<T> {
private:
  VEG_NODISCARD constexpr auto self() noexcept -> ::veg::option<T>&& {
    return static_cast<veg::option<T>&&>(*this);
  };

public:
  using option_copy_base<T>::option_copy_base;

  constexpr explicit operator bool() const noexcept {
    return this->is_engaged();
  }

  VEG_NODISCARD constexpr auto
  unwrap() && noexcept(meta::is_nothrow_constructible<
                       meta::remove_cv_t<T>,
                       meta::remove_cv_t<T>&&>::value) -> meta::remove_cv_t<T> {
    VEG_ASSERT(*this);
    return self().get();
  }
  VEG_NODISCARD constexpr auto take() noexcept(meta::is_nothrow_constructible<
                                               meta::remove_cv_t<T>,
                                               meta::remove_cv_t<T>&&>::value)
      -> option<T> {
    if (*this) {
      meta::remove_cv_t<T> val = self().get();
      this->destroy();
      return {some, VEG_MOV(val)};
    }
    return none;
  }

  VEG_TEMPLATE(
      (typename Fn),
      requires(meta::is_same<
               meta::detected_t<meta::invoke_result_t, Fn&&>,
               veg::option<T>>::value),
      constexpr auto or_else,
      (fn, Fn&&)) &&

      noexcept(
          (meta::is_nothrow_invocable<Fn&&>::value &&
           meta::is_nothrow_constructible<meta::remove_cv_t<T>, T const&>::
               value)) {
    return option_::or_else_impl<T>(self(), VEG_FWD(fn));
  }
};

template <typename T>
struct option_move_only_methods_base<T, true>
    : option_move_only_methods_base<T, false> {
private:
  VEG_NODISCARD constexpr auto self() const noexcept
      -> ::veg::option<T> const& {
    return static_cast<veg::option<T> const&>(*this);
  };
  using base = option_move_only_methods_base<T, false>;

public:
  using base::base;
  using base::or_else;
  using base::take;
  using base::unwrap;

  VEG_NODISCARD constexpr auto
  unwrap() const& noexcept(meta::is_nothrow_constructible<
                           meta::remove_cv_t<T>,
                           meta::remove_cv_t<T> const&>::value)
      -> meta::remove_cv_t<T> {
    return self().get();
  }

  VEG_TEMPLATE(
      (typename Fn),
      requires(meta::is_same<
               meta::detected_t<meta::invoke_result_t, Fn&&>,
               veg::option<T>>::value),
      constexpr auto or_else,
      (fn, Fn&&))
  const& noexcept(meta::is_nothrow_invocable<Fn&&>::value) {
    return option_::or_else_impl<T>(self(), VEG_FWD(fn));
  }
};

template <typename T, bool = meta::is_copy_constructible<T>::value>
struct option_base : option_move_only_methods_base<T> {
  using option_move_only_methods_base<T>::option_move_only_methods_base;
};

template <typename T>
struct option_base<option<T>, false>
    : option_move_only_methods_base<option<T>> {
  using option_move_only_methods_base<option<T>>::option_move_only_methods_base;
  constexpr auto
  flatten() && noexcept(meta::is_nothrow_constructible<T, T&&>::value)
      -> option<T> {
    if (*this) {
      return VEG_MOV(*this).unwrap();
    }
    return none;
  }
};
template <typename T>
struct option_base<option<T>, true> : option_base<option<T>, false> {
  using option_base<option<T>, false>::option_base;
  constexpr auto
  flatten() const& noexcept(meta::is_nothrow_constructible<T, T const&>::value)
      -> option<T> {
    if (*this) {
      return this->unwrap();
    }
    return none;
  }
};

} // namespace option_
} // namespace internal

template <typename T>
struct VEG_NODISCARD option : public internal::option_::option_base<T> {
  static_assert(meta::is_move_constructible<T>::value, "err");

  option() = default;
  ~option() = default;
  option(option const&) = default;
  option(option&&) noexcept(
      meta::is_nothrow_move_constructible<meta::remove_cv_t<T>>::value) =
      default;
  auto operator=(option const&) & -> option& = default;
  auto operator=(option&&) & noexcept(
      (meta::is_nothrow_move_constructible<meta::remove_cv_t<T>>::value &&
       meta::is_nothrow_move_assignable<meta::remove_cv_t<T>>::value))
      -> option& = default;

  explicit constexpr option(T value) noexcept(
      meta::is_nothrow_constructible<T, T&&>::value)
      : internal::option_::option_base<T>(VEG_FWD(value)) {}

  constexpr option(some_t /*tag*/, T value) noexcept(
      meta::is_nothrow_constructible<T, T&&>::value)
      : internal::option_::option_base<T>(VEG_FWD(value)) {}
  constexpr option // NOLINT(hicpp-explicit-conversions)
      (none_t /*tag*/) noexcept
      : option{} {}

  VEG_CVREF_DUPLICATE_TEMPLATE_2(
      option,
      (internal::option_::map_impl<
          T,
          meta::invoke_result_t<Fn&&, meta::collapse_category_t<T, Self&&>>>),
      (, VEG_FWD(fn)),

      (typename Fn),
      requires(meta::is_invocable<Fn&&, meta::collapse_category_t<T, Self&&>>::
                   value),
      VEG_NODISCARD constexpr auto map,
      (fn, Fn&&));

  VEG_CVREF_DUPLICATE_TEMPLATE_2(
      option,
      (internal::option_::and_then_impl<
          T,
          meta::invoke_result_t<Fn&&, meta::collapse_category_t<T, Self&&>>>),
      (, VEG_FWD(fn)),

      (typename Fn),
      requires(meta::is_option<meta::detected_t<
                   meta::invoke_result_t,
                   Fn&&,
                   meta::collapse_category_t<T, Self&&>>>::value),
      VEG_NODISCARD constexpr auto and_then,
      (fn, Fn&&));

  VEG_CVREF_DUPLICATE_TEMPLATE_2(
      option,
      (internal::option_::map_or_else_impl<
          T,
          meta::invoke_result_t<Fn&&, meta::collapse_category_t<T, Self&&>>>),
      (, VEG_FWD(fn), VEG_FWD(d)),

      (typename Fn, typename D),
      requires(
          (meta::is_invocable<D&&>::value && //
           meta::is_same<
               meta::detected_t<
                   meta::invoke_result_t,
                   Fn&&,
                   meta::collapse_category_t<T, Self&&>>,
               meta::detected_t<meta::invoke_result_t, D&&>>::value)),
      VEG_NODISCARD constexpr auto map_or_else,
      (fn, Fn&&),
      (d, D&&));

  template <typename Self, typename Fn, typename D>
  using map_ret = meta::detected_t<
      meta::invoke_result_t,
      Fn&&,
      meta::collapse_category_t<T, Self&&>>;

  VEG_CVREF_DUPLICATE_TEMPLATE_2(
      option,
      (internal::option_::map_or_impl<T, map_ret<Self, Fn, D>>),
      (, VEG_FWD(fn), VEG_FWD(d)),

      (typename Fn, typename D),
      requires(
          ((meta::is_reference<map_ret<Self, Fn, D>>::value &&
            meta::is_convertible<D, map_ret<Self, Fn, D>>::value) ||
           meta::is_constructible<map_ret<Self, Fn, D>, D&&>::value)),
      VEG_NODISCARD constexpr auto map_or,
      (fn, Fn&&),
      (d, D&&));

  VEG_CVREF_DUPLICATE(
      VEG_NODISCARD constexpr auto as_ref(),
      internal::option_::as_ref_impl<T>,
      ());

private:
  template <typename U, typename Self>
  friend constexpr auto internal::option_::as_ref_impl(Self&& self) noexcept
      -> option<meta::collapse_category_t<U, Self&&>>;

  template <typename U, internal::option_::kind K>
  friend struct internal::option_::value_sentinel_impl;
};

template <typename T>
struct meta::value_sentinel_for<option<T>>
    : veg::internal::option_::value_sentinel_impl<T> {};

} // namespace veg

#endif /* end of include guard VEG_OPTION_HPP_8NVLXES2S */
