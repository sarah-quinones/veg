#ifndef VEG_STORAGE_HPP_X0B4XDKES
#define VEG_STORAGE_HPP_X0B4XDKES

#include "veg/internal/type_traits.hpp"
#include "veg/internal/memory.hpp"

namespace veg {
namespace internal {
namespace storage {

template <bool>
struct delete_copy_ctor_if {
  delete_copy_ctor_if() = default;
  ~delete_copy_ctor_if() = default;
  delete_copy_ctor_if(delete_copy_ctor_if const&) = delete;
  delete_copy_ctor_if(delete_copy_ctor_if&&) noexcept = default;
  auto operator=(delete_copy_ctor_if const&) -> delete_copy_ctor_if& = default;
  auto operator=(delete_copy_ctor_if&&) noexcept
      -> delete_copy_ctor_if& = default;
};
template <>
struct delete_copy_ctor_if<false> {};

template <bool>
struct delete_copy_assign_if {
  delete_copy_assign_if() = default;
  ~delete_copy_assign_if() = default;
  delete_copy_assign_if(delete_copy_assign_if const&) = default;
  delete_copy_assign_if(delete_copy_assign_if&&) noexcept = default;
  auto operator=(delete_copy_assign_if const&)
      -> delete_copy_assign_if& = delete;
  auto operator=(delete_copy_assign_if&&) noexcept
      -> delete_copy_assign_if& = default;
};
template <>
struct delete_copy_assign_if<false> {};

template <bool>
struct delete_move_ctor_if {
  delete_move_ctor_if() = default;
  ~delete_move_ctor_if() = default;
  delete_move_ctor_if(delete_move_ctor_if const&) = default;
  delete_move_ctor_if(delete_move_ctor_if&&) noexcept = delete;
  auto operator=(delete_move_ctor_if const&) -> delete_move_ctor_if& = default;
  auto operator=(delete_move_ctor_if&&) noexcept
      -> delete_move_ctor_if& = default;
};
template <>
struct delete_move_ctor_if<false> {};

template <bool>
struct delete_move_assign_if {
  delete_move_assign_if() = default;
  ~delete_move_assign_if() = default;
  delete_move_assign_if(delete_move_assign_if const&) = default;
  delete_move_assign_if(delete_move_assign_if&&) noexcept = default;
  auto operator=(delete_move_assign_if const&)
      -> delete_move_assign_if& = default;
  auto operator=(delete_move_assign_if&&) noexcept
      -> delete_move_assign_if& = delete;
};
template <>
struct delete_move_assign_if<false> {};

template <typename T, bool = meta::is_default_constructible<T>::value>
struct storage_base {
  using type = std::remove_const_t<T>;
  type inner_val = {};
  constexpr storage_base() = default;
  explicit constexpr storage_base(type&& arg)
      : inner_val{static_cast<type&&>(arg)} {}
};

template <typename T>
struct storage_base<T, false> {
  using type = std::remove_const_t<T>;
  type inner_val;
  explicit constexpr storage_base(meta::remove_cvref_t<T>&& arg)
      : inner_val{static_cast<meta::remove_cvref_t<T>&&>(arg)} {}
};

template <typename T>
struct storage : storage_base<T> {
  using storage_base<T>::storage_base;
  using storage_base<T>::inner_val;

  using typename storage_base<T>::type;

  auto get() const noexcept -> T const& { return inner_val; }
  auto get_mut() noexcept -> T& { return inner_val; }
  auto get_mov_ref() && noexcept -> decltype(auto) {
    return static_cast<type&&>(inner_val);
  }
  auto get_mov() && noexcept -> auto { return static_cast<type&&>(inner_val); }

  VEG_TEMPLATE(
      (typename V),
      requires(
          meta::is_swappable<T&, V&>::value && //
          !std::is_reference<V>::value),
      friend constexpr void swap,
      (t, storage<T>&),
      (v, storage<V>&))
  noexcept(meta::is_nothrow_swappable<T&, V&>::value) {
    fn::swap_fn{}(t.get_mut(), v.get_mut());
  }
};

template <typename T>
struct storage<T&> {
  using type = T&;
  T* inner_ptr = nullptr;

  explicit constexpr storage(T& arg) noexcept
      : inner_ptr{mem::addressof(arg)} {}

  auto get() const noexcept -> T& { return *inner_ptr; }
  auto get_mut() noexcept -> T& { return *inner_ptr; }
  auto get_mov_ref() && noexcept -> T& { return *inner_ptr; }
  auto get_mov() && noexcept -> T& { return *inner_ptr; }

  friend constexpr void swap(storage& u, storage& v) noexcept {
    swap_::mov_fn_swap::apply(u.inner_ptr, v.inner_ptr);
  }
};

template <typename T>
struct storage<T&&> : delete_copy_ctor_if<true>, delete_copy_assign_if<true> {
  using type = T&&;
  T* inner_ptr = nullptr;
  explicit constexpr storage(T&& arg) noexcept
      : inner_ptr{mem::addressof(arg)} {}
  auto get() const noexcept -> T& { return *inner_ptr; }
  auto get_mut() noexcept -> T& { return *inner_ptr; }
  auto get_mov_ref() && noexcept -> T&& { return static_cast<T&&>(*inner_ptr); }
  auto get_mov() && noexcept -> T&& { return static_cast<T&&>(*inner_ptr); }
  friend constexpr void swap(storage& u, storage& v) noexcept {
    swap_::mov_fn_swap::apply(u.inner_ptr, v.inner_ptr);
  }
};

// intentionally unsupported. please don't
template <typename T>
struct storage<T const&&>;

template <meta::category_e C>
struct get_inner;

template <>
struct get_inner<meta::category_e::own> {
  template <typename T>
  static constexpr auto apply(storage<T>&& arg) -> decltype(auto) {
    return static_cast<storage<T>&&>(arg).get_mov();
  }

  template <typename Idx, template <Idx, typename> class Indexed>
  struct with_idx {
    template <Idx I, typename T>
    static constexpr auto get_type(Indexed<I, T> const& arg) -> T;
    template <Idx I, typename T>
    static constexpr auto apply(Indexed<I, T>&& arg) -> decltype(auto) {
      return static_cast<storage<T>&&>(arg).get_mov();
    }
  };
};

template <>
struct get_inner<meta::category_e::ref> {
  template <typename T>
  static constexpr auto apply(storage<T> const& arg) noexcept
      -> decltype(auto) {
    return arg.get();
  }

  template <typename Idx, template <Idx, typename> class Indexed>
  struct with_idx {
    template <Idx I, typename T>
    static constexpr auto apply(Indexed<I, T> const& arg) noexcept
        -> decltype(auto) {
      return arg.get();
    }

    template <Idx I, typename T>
    static constexpr auto impl(Indexed<I, T> const& arg) noexcept -> auto& {
      return arg;
    }
  };
};
template <>
struct get_inner<meta::category_e::ref_mut> {
  template <typename T>
  static constexpr auto apply(storage<T>& arg) noexcept -> decltype(auto) {
    return arg.get_mut();
  }

  template <typename Idx, template <Idx, typename> class Indexed>
  struct with_idx {
    template <Idx I, typename T>
    static constexpr auto apply(Indexed<I, T>& arg) noexcept -> decltype(auto) {
      return arg.get_mut();
    }

    template <Idx I, typename T>
    static constexpr auto impl(Indexed<I, T>& arg) noexcept -> auto& {
      return arg;
    }
  };
};
template <>
struct get_inner<meta::category_e::ref_mov> {
  template <typename T>
  static constexpr auto apply(storage<T>&& arg) noexcept -> decltype(auto) {
    return static_cast<storage<T>&&>(arg).get_mov_ref();
  }

  template <typename Idx, template <Idx, typename> class Indexed>
  struct with_idx {
    template <Idx I, typename T>
    static constexpr auto apply(Indexed<I, T>&& arg) noexcept
        -> decltype(auto) {
      return static_cast<storage<T>&&>(arg).get_mov_ref();
    }
  };
};

} // namespace storage
} // namespace internal
} // namespace veg

#endif /* end of include guard VEG_STORAGE_HPP_X0B4XDKES */
