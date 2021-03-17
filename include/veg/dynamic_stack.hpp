#ifndef DYNAMIC_STACK_DYNAMIC_STACK_HPP_UBOMZFTOS
#define DYNAMIC_STACK_DYNAMIC_STACK_HPP_UBOMZFTOS

#include "veg/internal/meta_int.hpp"
#include "veg/internal/type_traits.hpp"
#include "veg/assert.hpp"
#include "veg/slice.hpp"
#include "veg/internal/memory.hpp"
#include "veg/option.hpp"
#include "veg/internal/narrow.hpp"

namespace veg {
namespace internal {
namespace dynstack {

struct cleanup;
struct dynamic_alloc_base;

auto align_next(i64 alignment, i64 size, void*& ptr, i64& space) noexcept
    -> void*;

struct default_init_fn {
  template <typename T>
  auto make(void* ptr, i64 len) -> T* {
    return new (ptr) T[fn::narrow<usize>{}(len)];
  }
};

struct zero_init_fn {
  template <typename T>
  auto make(void* ptr, i64 len) -> T* {
    return new (ptr) T[fn::narrow<usize>{}(len)]{};
  }
};

struct no_init_fn {
  template <typename T>
  auto make(void* ptr, i64 len) -> T* {
    return VEG_LAUNDER(static_cast<T*>(static_cast<void*>(
        new (ptr) unsigned char[fn::narrow<usize>{}(len) * sizeof(T)])));
  }
};

} // namespace dynstack
} // namespace internal

template <typename T>
struct dynstack_array;
template <typename T>
struct dynstack_alloc;

struct dynamic_stack_view {
public:
  dynamic_stack_view /* NOLINT(hicpp-explicit-conversions) */ (
      slice<void> s) noexcept
      : stack_data(s.data()), stack_bytes(s.size()) {}

  VEG_NODISCARD
  auto remaining_bytes() const noexcept -> i64 { return stack_bytes; }

  VEG_TEMPLATE(
      (typename T),
      requires __VEG_CONCEPT(meta::constructible<T>),
      VEG_NODISCARD auto make_new,
      (/*unused*/, tag_t<T>),
      (len, i64),
      (align = alignof(T), i64))
  noexcept(__VEG_CONCEPT(meta::nothrow_constructible<T>))
      -> option<dynstack_array<T>> {
    dynstack_array<T> get{
        *this, len, align, internal::dynstack::zero_init_fn{}};
    if (get.data() == nullptr) {
      return none;
    }
    return {some, VEG_FWD(get)};
  }

  VEG_TEMPLATE(
      (typename T),
      requires __VEG_CONCEPT(meta::constructible<T>),
      VEG_NODISCARD auto make_new_for_overwrite,
      (/*unused*/, tag_t<T>),
      (len, i64),
      (align = alignof(T), i64))

  noexcept(__VEG_CONCEPT(meta::nothrow_constructible<T>))
      -> option<dynstack_array<T>> {
    dynstack_array<T> get{
        *this, len, align, internal::dynstack::default_init_fn{}};
    if (get.data() == nullptr) {
      return none;
    }
    return {some, VEG_FWD(get)};
  }

  template <typename T>
  VEG_NODISCARD auto
  make_alloc(tag_t<T> /*unused*/, i64 len, i64 align = alignof(T)) noexcept
      -> option<dynstack_alloc<T>> {
    dynstack_alloc<T> get{*this, len, align, internal::dynstack::no_init_fn{}};
    if (get.data() == nullptr) {
      return none;
    }
    return {some, VEG_FWD(get)};
  }

private:
  void* stack_data;
  i64 stack_bytes;

  template <typename T>
  friend struct dynstack_alloc;
  template <typename T>
  friend struct dynstack_array;
  friend struct internal::dynstack::cleanup;
  friend struct internal::dynstack::dynamic_alloc_base;
};

namespace internal {
namespace dynstack {

struct cleanup {
  bool const& success;
  dynamic_stack_view& parent;
  void* old_data;
  i64 old_rem_bytes;

  HEDLEY_ALWAYS_INLINE void operator()() const noexcept {
    if (!success) {
      parent.stack_data = old_data;
      parent.stack_bytes = old_rem_bytes;
    }
  }
};

struct dynamic_alloc_base {
  dynamic_stack_view& parent;
  void* old_pos;
  void const volatile* data = nullptr;
  i64 len = 0;

  void destroy(void const volatile* data_end) noexcept {
    if (len != 0) {
      // in case resource lifetimes are reodered by moving ownership
      VEG_ASSERT(parent.stack_data == data_end);
      VEG_ASSERT(
          static_cast<unsigned char*>(parent.stack_data) >=
          static_cast<unsigned char*>(old_pos));

      parent.stack_bytes += fn::narrow<i64>{}(
          static_cast<unsigned char*>(parent.stack_data) -
          static_cast<unsigned char*>(old_pos));
      parent.stack_data = old_pos;
    }
  }
};
} // namespace dynstack
} // namespace internal

template <typename T>
struct dynstack_alloc : internal::dynstack::dynamic_alloc_base {
private:
  using base = internal::dynstack::dynamic_alloc_base;

public:
  ~dynstack_alloc() { base::destroy(data() + base::len); }
  dynstack_alloc(dynstack_alloc const&) = delete;
  dynstack_alloc(dynstack_alloc&& other) noexcept : base{base(other)} {
    other.base::len = 0;
    other.base::data = nullptr;
  };

  auto operator=(dynstack_alloc const&) -> dynstack_alloc& = delete;
  auto operator=(dynstack_alloc&&) -> dynstack_alloc& = delete;

  VEG_NODISCARD auto data() const noexcept -> T* {
    return static_cast<T*>(const_cast<void*>(base::data));
  }
  VEG_NODISCARD auto size() const noexcept -> i64 { return base::len; }

private:
  friend struct dynstack_array<T>;
  friend struct dynamic_stack_view;

  template <typename Fn>
  dynstack_alloc(
      dynamic_stack_view& parent,
      i64 alloc_size,
      i64 align,
      Fn fn) noexcept(__VEG_CONCEPT(meta::nothrow_constructible<T>))
      : base{parent, parent.stack_data} {

    void* const parent_data = parent.stack_data;
    i64 const parent_bytes = parent.stack_bytes;

    void* const data = internal::dynstack::align_next(
        align,
        alloc_size * fn::narrow<i64>{}(sizeof(T)),
        parent.stack_data,
        parent.stack_bytes);

    if (data != nullptr) {
      bool success = false;
      auto&& cleanup = make::defer(internal::dynstack::cleanup{
          success, parent, parent_data, parent_bytes});
      (void)cleanup;

      base::len = alloc_size;
      base::data = fn.template make<T>(data, alloc_size);

      success = true;
    }
  }
};

template <typename T>
struct dynstack_array : private dynstack_alloc<T> {

public:
  using dynstack_alloc<T>::data;
  using dynstack_alloc<T>::size;

  dynstack_array(dynstack_array const&) = delete;
  dynstack_array(dynstack_array&&) noexcept = default;
  auto operator=(dynstack_array const&) -> dynstack_array& = delete;
  auto operator=(dynstack_array&&) -> dynstack_array& = delete;

  ~dynstack_array() {
    for (i64 i = this->base::len - 1; i >= 0; --i) {
      fn::destroy_at{}(this->data() + i);
    }
  }

private:
  using dynstack_alloc<T>::dynstack_alloc;
  friend struct dynamic_stack_view;
};

} // namespace veg

#endif /* end of include guard DYNAMIC_STACK_DYNAMIC_STACK_HPP_UBOMZFTOS */
