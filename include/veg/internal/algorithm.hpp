#ifndef VEG_ALGORITHM_HPP_PLKQ80IYS
#define VEG_ALGORITHM_HPP_PLKQ80IYS

#include "veg/util/defer.hpp"
#include "veg/memory/aligned_alloc.hpp"
#include "veg/memory/address.hpp"
#include "veg/memory/placement.hpp"
#include "veg/memory/aligned_alloc.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
inline namespace VEG_ABI {
namespace internal {
namespace algo_ {

template <typename T, typename... Args>
HEDLEY_ALWAYS_INLINE
VEG_CPP20(constexpr) void backward_destroy_n(T* ptr, i64 n) noexcept {
	if (n == 0) {
		return;
	}
	for (T* p = ptr + n - 1;;) {
		mem::destroy_at(p);

		if (p == ptr) {
			break;
		}
		--p;
	}
}

template <typename T>
struct destroy_range_fn {
	T* cleanup_ptr;
	i64 const& size;

	VEG_CPP20(constexpr)
	void operator()() const noexcept {
		algo_::backward_destroy_n(cleanup_ptr, size);
	}
};

template <typename Cast, typename T, typename... Args>
VEG_CPP20(constexpr)
void uninit_emplace_n(T* dest, T const* src, i64 n) noexcept(
		VEG_CONCEPT(nothrow_constructible<T, Cast>)) {

	i64 i = 0;
	auto&& cleanup = make::defer(internal::algo_::destroy_range_fn<T>{dest, i});
	for (; i < n; ++i) {
		mem::construct_at(dest + i, const_cast<Cast>(src[i]));
	}
	i = 0;
}

template <typename Cast, typename T>
VEG_CPP20(constexpr)
void reloc_fallible(T* dest, T* src, i64 n) noexcept(
		VEG_CONCEPT(nothrow_constructible<T, Cast>)) {

	i64 i = 0;
	auto&& cleanup = make::defer(destroy_range_fn<T>{dest, i});
	for (; i < n; ++i) {
		mem::construct_at(dest + i, static_cast<Cast>(src[i]));
	}
	cleanup.fn.cleanup_ptr = src;
}

enum struct which {
	trivial,
	nothrow_move,
	copy,
	throw_move,
};

template <which Which>
struct reloc_impl;

template <>
struct reloc_impl<which::nothrow_move> {
	template <typename T>
	static VEG_CPP20(constexpr) void apply(T* dest, T* src, i64 n) noexcept {
		T* end = dest + n;
		for (; dest < end; ++dest, ++src) {
			mem::construct_at(dest, static_cast<T&&>(*src));
			mem::destroy_at(*src);
		}
	}
};

template <>
struct reloc_impl<which::trivial> {
	template <typename T>
	static VEG_CPP20(constexpr) void apply(T* dest, T* src, i64 n) noexcept {
		static_assert(
				VEG_CONCEPT(nothrow_move_constructible<T>),
				"is T really trivially relocatable?");

		VEG_CPP20(

				if (std::is_constant_evaluated()) {
					reloc_impl<which::nothrow_move>::apply(dest, src, n);
				} else

		)

		{
			abi::internal::opaque_memmove(
					dest, src, static_cast<usize>(n) * sizeof(T));
		}
	}
};

template <>
struct reloc_impl<which::copy> {
	template <typename T>
	static VEG_CPP20(constexpr) void apply(T* dest, T* src, i64 n) noexcept(
			VEG_CONCEPT(nothrow_constructible<T, T const&>)) {
		algo_::reloc_fallible<T const&>(dest, src, n);
	}
};

template <>
struct reloc_impl<which::throw_move> {
	template <typename T>
	HEDLEY_ALWAYS_INLINE static VEG_CPP20(constexpr) void apply(
			T* dest, T* src, i64 n) //
			noexcept(false) {
		algo_::reloc_fallible<T&&>(dest, src, n);
	}
};

} // namespace algo_
} // namespace internal

namespace fn {

struct backward_destroy_n {
	VEG_TEMPLATE(
			(typename T, typename... Args),
			requires !VEG_CONCEPT(void_type<T>),
			HEDLEY_ALWAYS_INLINE VEG_CPP20(constexpr) void
			operator(),
			(ptr, T*),
			(n, i64))
	const noexcept {
		if (n == 0) {
			return;
		}
		for (T* p = ptr + n - 1;; --p) {
			mem::destroy_at(p);

			if (p == ptr) {
				break;
			}
		}
	}
};

struct relocate_n {
	VEG_TEMPLATE(
			(typename T),
			requires(
					!VEG_CONCEPT(const_type<T>) && (VEG_CONCEPT(move_constructible<T>) ||
	                                        VEG_CONCEPT(copy_constructible<T>))),
			HEDLEY_ALWAYS_INLINE VEG_CPP20(constexpr) void
			operator(),
			(dest, T*),
			(src, T*),
			(n, i64))
	const noexcept(
			VEG_CONCEPT(nothrow_move_constructible<T>) ||
			VEG_CONCEPT(nothrow_constructible<T, T const&>)) {

		namespace impl = internal::algo_;
		impl::reloc_impl<
				VEG_CONCEPT(trivially_relocatable<T>)            //
						? impl::which::trivial                       //
						: VEG_CONCEPT(nothrow_move_constructible<T>) //
									? impl::which::nothrow_move            //
									: VEG_CONCEPT(copy_constructible<T>)   //
												? impl::which::copy              //
												: impl::which::throw_move        //
				>::apply(dest, src, n);
	}
};

struct uninitialized_move_n {
	VEG_TEMPLATE(
			(typename T),
			requires VEG_CONCEPT(move_constructible<T>),
			HEDLEY_ALWAYS_INLINE VEG_CPP20(constexpr) void
			operator(),
			(dest, T*),
			(src, T*),
			(n, i64))
	const noexcept(VEG_CONCEPT(nothrow_move_constructible<T>)) {
		internal::algo_::uninit_emplace_n<T&&>(dest, src, n);
	}
};

struct uninitialized_copy_n {
	VEG_TEMPLATE(
			(typename T, int = 0),
			requires VEG_CONCEPT(trivially_copyable<T>) &&
					VEG_CONCEPT(copy_constructible<T>),
			HEDLEY_ALWAYS_INLINE VEG_CPP20(constexpr) void
			operator(),
			(dest, T*),
			(src, T const*),
			(n, i64))
	const noexcept(VEG_CONCEPT(nothrow_copy_constructible<T>)) {
		abi::internal::opaque_memmove(dest, src, sizeof(T) * static_cast<usize>(n));
	}
	VEG_TEMPLATE(
			(typename T),
			requires !VEG_CONCEPT(trivially_copyable<T>) &&
					VEG_CONCEPT(copy_constructible<T>),
			HEDLEY_ALWAYS_INLINE VEG_CPP20(constexpr) void
			operator(),
			(dest, T*),
			(src, T const*),
			(n, i64))
	const noexcept(VEG_CONCEPT(nothrow_copy_constructible<T>)) {
		internal::algo_::uninit_emplace_n<T const&>(dest, src, n);
	}
};

struct copy_n {
	VEG_TEMPLATE(
			(typename T),
			requires VEG_CONCEPT(copy_assignable<T>),
			HEDLEY_ALWAYS_INLINE VEG_CPP20(constexpr) void
			operator(),
			(dest, T*),
			(src, T const*),
			(n, i64))
	const noexcept(VEG_CONCEPT(nothrow_copy_assignable<T>)) {
		for (i64 i = 0; i < n; ++i) {
			dest[i] = src[i];
		}
	}
};

struct mixed_init_copy_n {
	VEG_TEMPLATE(
			(typename T, int = 0),
			requires VEG_CONCEPT(trivially_copyable<T>) &&
					VEG_CONCEPT(copy_assignable<T>) && VEG_CONCEPT(copy_constructible<T>),
			HEDLEY_ALWAYS_INLINE VEG_CPP20(constexpr) void
			operator(),
			(dest, T*),
			(src, T const*),
			(n, i64),
			(n_init, i64))
	const noexcept {
		(void)n_init;
		abi::internal::opaque_memmove(dest, src, sizeof(T) * static_cast<usize>(n));
	}

	VEG_TEMPLATE(
			(typename T),
			requires !VEG_CONCEPT(trivially_copyable<T>) &&
					VEG_CONCEPT(copy_assignable<T>) && VEG_CONCEPT(copy_constructible<T>),
			HEDLEY_ALWAYS_INLINE VEG_CPP20(constexpr) void
			operator(),
			(dest, T*),
			(src, T const*),
			(n, i64),
			(n_init, i64))
	const noexcept(
			VEG_CONCEPT(nothrow_copy_assignable<T>) &&
			VEG_CONCEPT(copy_constructible<T>)) {
		n_init = n < n_init ? n : n_init;
		copy_n{}(dest, src, n_init);
		uninitialized_copy_n{}(dest + n_init, src + n_init, n - n_init);
	}
};

} // namespace fn
VEG_INLINE_VAR(backward_destroy_n, fn::backward_destroy_n);
VEG_INLINE_VAR(relocate_n, fn::relocate_n);
VEG_INLINE_VAR(uninitialized_move_n, fn::uninitialized_move_n);
VEG_INLINE_VAR(uninitialized_copy_n, fn::uninitialized_copy_n);
VEG_INLINE_VAR(copy_n, fn::copy_n);
VEG_INLINE_VAR(mixed_init_copy_n, fn::mixed_init_copy_n);

namespace internal {
namespace algo_ {

struct free_cleanup {
	void* ptr;
	i64 cap_bytes;
	auto operator()() const noexcept { mem::aligned_free(ptr, cap_bytes); }
};

template <typename T>
HEDLEY_ALWAYS_INLINE auto reallocate_memory(
		T* src, i64 align, i64 size, i64 cap, i64 new_cap) noexcept(false) -> T* {

	constexpr i64 s = static_cast<i64>(sizeof(T));
	T* p = static_cast<T*>(mem::aligned_alloc(align, new_cap * s));
	auto&& cleanup = make::defer(free_cleanup{p, new_cap * s});

	relocate_n(p, src, size);

	cleanup.fn.ptr = src;
	cleanup.fn.cap_bytes = cap * s;

	return p;
}

} // namespace algo_
} // namespace internal
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_ALGORITHM_HPP_PLKQ80IYS */
