#ifndef VEG_STORAGE_HPP_X0B4XDKES
#define VEG_STORAGE_HPP_X0B4XDKES

#include "veg/internal/terminate.hpp"
#include "veg/memory/address.hpp"
#include "veg/type_traits/cpo/tombstone.hpp"
#include "veg/type_traits/constructible.hpp"
#include "veg/type_traits/assignable.hpp"
#include "veg/internal/delete_special_members.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
inline namespace VEG_ABI {
namespace internal {
namespace storage_ {

template <typename T>
struct storage;

template <typename T>
struct storage<T&> {
	T* inner_ptr;

	storage() = default;
	HEDLEY_ALWAYS_INLINE constexpr storage(T& arg) noexcept
			: inner_ptr{mem::addressof(arg)} {}

	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto operator=(T& arg) noexcept
			-> storage& {
		inner_ptr = mem::addressof(arg);
		return *this;
	}
	HEDLEY_ALWAYS_INLINE constexpr operator T&() const noexcept {
		return *inner_ptr;
	}
};

template <typename T>
struct storage<T&&> : internal::nocopy_ctor, internal::nocopy_assign {
	T* inner_ptr;

	storage() = default;
	HEDLEY_ALWAYS_INLINE constexpr storage(T&& arg) noexcept
			: inner_ptr{mem::addressof(arg)} {}

	HEDLEY_ALWAYS_INLINE VEG_CPP14(constexpr) auto operator=(T&& arg) noexcept
			-> storage& {
		inner_ptr = mem::addressof(arg);
		return *this;
	}
	HEDLEY_ALWAYS_INLINE constexpr operator T&&() const&& noexcept {
		return static_cast<T&&>(*inner_ptr);
	}
	HEDLEY_ALWAYS_INLINE constexpr operator T&() const& noexcept {
		return *inner_ptr;
	}
};

// intentionally unsupported. please don't
template <typename T>
struct storage<T const&&>;

} // namespace storage_
} // namespace internal

template <typename T>
struct meta::tombstone_traits<veg::internal::storage_::storage<T>> {
	static constexpr i64 spare_representations = 1;

	HEDLEY_ALWAYS_INLINE static VEG_CPP14(constexpr) void set_spare_representation(
			internal::storage_::storage<T>* p, i64 /*i*/) noexcept {
		p->inner_ptr = nullptr;
	}
	HEDLEY_ALWAYS_INLINE static VEG_CPP14(constexpr) auto index(
			internal::storage_::storage<T> const* p) noexcept -> i64 {
		return (p->inner_ptr == nullptr) ? 0 : -1;
	}
};
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_STORAGE_HPP_X0B4XDKES */
