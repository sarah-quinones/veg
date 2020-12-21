#ifndef DYNAMIC_STACK_DYNAMIC_STACK_HPP_UBOMZFTOS
#define DYNAMIC_STACK_DYNAMIC_STACK_HPP_UBOMZFTOS

#include "veg/internal/meta_int.hpp"
#include "veg/internal/type_traits.hpp"
#include "veg/assert.hpp"
#include "veg/slice.hpp"
#include "veg/internal/memory.hpp"

namespace veg {
namespace internal {
namespace dynstack {

auto align_next(i64 alignment, i64 size, void*& ptr, i64& space) noexcept
    -> void*;

struct default_init_fn {
  template <typename T>
  auto make(void* ptr, i64 len) -> T* {
    return new (ptr) T[narrow<usize>(len)];
  }
};

struct zero_init_fn {
  template <typename T>
  auto make(void* ptr, i64 len) -> T* {
    return new (ptr) T[narrow<usize>(len)]{};
  }
};

struct no_init_fn {
  template <typename T>
  auto make(void* ptr, i64 len) -> T* {
    return VEG_LAUNDER(static_cast<T*>(static_cast<void*>(
        new (ptr) unsigned char[narrow<usize>(len) * sizeof(T)])));
  }
};

} // namespace dynstack
} // namespace internal

template <typename T>
struct dynamic_array;
template <typename T>
struct dynamic_alloc;

struct dynamic_stack_view {
public:
  dynamic_stack_view /* NOLINT(hicpp-explicit-conversions) */ (
      slice<void> s) noexcept
      : m_data(s.data()), m_rem_bytes(s.size()) {}

  VEG_NODISCARD
  auto remaining_bytes() const noexcept -> i64 { return m_rem_bytes; }

  VEG_TEMPLATE(
      (typename T),
      requires(
          (std::is_default_constructible<T>::value &&
           std::is_destructible<T>::value)),
      VEG_NODISCARD auto make_new,
      (/*unused*/, tag_t<T>),
      (len, i64),
      (align = alignof(T), i64))
  noexcept(std::is_nothrow_default_constructible<T>::value)
      -> dynamic_array<T> {
    return {*this, len, align, internal::dynstack::zero_init_fn{}};
  }

  VEG_TEMPLATE(
      (typename T),
      requires(
          (std::is_default_constructible<T>::value &&
           std::is_destructible<T>::value)),
      VEG_NODISCARD auto make_new_for_overwrite,
      (/*unused*/, tag_t<T>),
      (len, i64),
      (align = alignof(T), i64))

  noexcept(std::is_nothrow_default_constructible<T>::value)
      -> dynamic_array<T> {
    return {*this, len, align, internal::dynstack::default_init_fn{}};
  }

  template <typename T>
  VEG_NODISCARD auto
  make_alloc(tag_t<T> /*unused*/, i64 len, i64 align = alignof(T)) noexcept
      -> dynamic_alloc<T> {
    return {*this, len, align, internal::dynstack::no_init_fn{}};
  }

private:
  void* m_data;
  i64 m_rem_bytes;

  template <typename T>
  friend struct dynamic_alloc;
  template <typename T>
  friend struct dynamic_array;
};

template <typename T>
struct dynamic_alloc {

public:
  ~dynamic_alloc() = default;
  dynamic_alloc(dynamic_alloc const&) = delete;
  dynamic_alloc(dynamic_alloc&& other) noexcept
      : m_parent(other.m_parent),
        m_old_pos(other.m_old_pos),
        m_data(other.m_data),
        m_len(other.m_len) {
    other.m_len = 0;
    other.m_data = nullptr;
  };

  auto operator=(dynamic_alloc const&) -> dynamic_alloc& = delete;
  auto operator=(dynamic_alloc&&) -> dynamic_alloc& = delete;

  VEG_NODISCARD auto data() const noexcept -> T* { return m_data; }
  VEG_NODISCARD auto size() const noexcept -> i64 { return m_len; }

private:
  dynamic_stack_view& m_parent;
  void* m_old_pos;
  T* m_data = nullptr;
  i64 m_len = 0;

  friend struct dynamic_array<T>;
  friend struct dynamic_stack_view;

  template <typename Fn>
  dynamic_alloc(dynamic_stack_view& parent, i64 len, i64 align, Fn fn) noexcept(
      noexcept(T()))
      : m_parent(parent), m_old_pos(parent.m_data) {

    void* data = internal::dynstack::align_next(
        align,
        len * narrow<i64>(sizeof(T)),
        m_parent.m_data,
        m_parent.m_rem_bytes);

    if (data != nullptr) {
      m_len = len;
      m_data = fn.template make<T>(data, len);
    }
  }
};

template <typename T>
struct dynamic_array : private dynamic_alloc<T> {

public:
  using dynamic_alloc<T>::data;
  using dynamic_alloc<T>::size;

  dynamic_array(dynamic_array const&) = delete;
  dynamic_array(dynamic_array&&) noexcept = default;
  auto operator=(dynamic_array const&) -> dynamic_array& = delete;
  auto operator=(dynamic_array&&) -> dynamic_array& = delete;

  ~dynamic_array() {
    if (this->m_len != 0) {
      // in case resource lifetimes are reodered by moving ownership
      VEG_ASSERT(this->m_parent.m_data == (this->m_data + this->m_len));

      if (this->m_parent.m_data != (this->m_data + this->m_len)) {
        VEG_ASSERT(
            static_cast<unsigned char*>(this->m_parent.m_data) >=
            static_cast<unsigned char*>(this->m_old_pos));
      }
      for (i64 i = 0; i < this->m_len; ++i) {
        (this->m_data + i)->~T();
      }
      this->m_parent.m_rem_bytes += narrow<i64>(
          static_cast<unsigned char*>(this->m_parent.m_data) -
          static_cast<unsigned char*>(this->m_old_pos));
      this->m_parent.m_data = this->m_old_pos;
    }
  }

private:
  using dynamic_alloc<T>::dynamic_alloc;
  friend struct dynamic_stack_view;
};

} // namespace veg

#endif /* end of include guard DYNAMIC_STACK_DYNAMIC_STACK_HPP_UBOMZFTOS */
