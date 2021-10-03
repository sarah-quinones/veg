#ifndef VEG_ALGORITHM_HPP_PLKQ80IYS
#define VEG_ALGORITHM_HPP_PLKQ80IYS

#include "veg/util/defer.hpp"
#include "veg/memory/alloc.hpp"
#include "veg/memory/address.hpp"
#include "veg/memory/placement.hpp"
#include "veg/type_traits/assignable.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
namespace internal {
namespace algo_ {

template <typename T, typename Fn>
VEG_INLINE VEG_CPP20(constexpr) void backward_destroy_n_direct(
		T* ptr, usize n, Fn const fn)
		VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_fn<Fn, void, T*>)) {
	if (n == 0) {
		return;
	}
	for (T* p = ptr + n - 1;;) {
		fn(static_cast<T*>(p));

		if (p == ptr) {
			break;
		}
		--p;
	}
}

template <typename T, typename Fn>
struct DestroyRangeFn {
	T* cleanup_ptr;
	usize const& size;
	Fn fn;

	VEG_CPP20(constexpr)
	VEG_INLINE void operator()() const VEG_NOEXCEPT {
		algo_::backward_destroy_n_direct(cleanup_ptr, size, fn);
	}
};

template <typename T, typename Fn>
VEG_INLINE VEG_CPP20(constexpr) void backward_destroy_n_may_throw(
		T* ptr, usize n, Fn const fn)
		VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_fn<Fn, void, T*>)) {
	if (n == 0) {
		return;
	}
	usize i = n - 1;
	auto&& cleanup = defer(DestroyRangeFn<T, Fn>{ptr, i, fn});
	(void)cleanup;

	while (true) {
		fn(static_cast<T*>(ptr + i));
		if (i == 0) {
			break;
		}
		--i;
	}
	i = 0;
}

template <typename Cast, typename T, typename... Args>
VEG_CPP20(constexpr)
void uninit_emplace_n(T* dest, T const* src, usize n)
		VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_constructible<T, Cast>)) {

	usize i = 0;
	auto&& cleanup = defer(
			internal::algo_::DestroyRangeFn<T, mem::nb::destroy_at>{dest, i, {}});
	for (; i < n; ++i) {
		mem::construct_at(dest + i, const_cast<Cast>(src[i]));
	}
	i = 0;
}

template <typename Cast, typename T>
VEG_CPP20(constexpr)
void reloc_fallible(T* dest, T* src, usize n)
		VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_constructible<T, Cast>)) {

	usize i = 0;
	auto&& cleanup = defer(DestroyRangeFn<T, mem::nb::destroy_at>{dest, i, {}});
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
	static VEG_CPP20(constexpr) void apply(T* dest, T* src, usize n)
			VEG_NOEXCEPT {
		T* end = dest + n;
		for (; dest < end; ++dest, ++src) {
			mem::construct_at(dest, static_cast<T&&>(*src));
			mem::destroy_at(src);
		}
	}
};

template <>
struct reloc_impl<which::trivial> {
	template <typename T>
	static VEG_CPP20(constexpr) void apply(T* dest, T* src, usize n)
			VEG_NOEXCEPT {
		static_assert(
				VEG_CONCEPT(nothrow_movable<T>), "is T really trivially relocatable?");

		VEG_CPP20(

				if (std::is_constant_evaluated()) {
					reloc_impl<which::nothrow_move>::apply(dest, src, n);
				} else

		)

		{
			abi::internal::veglib_opaque_memmove(
					dest, src, static_cast<usize>(n) * sizeof(T));
		}
	}
};

template <>
struct reloc_impl<which::copy> {
	template <typename T>
	static VEG_CPP20(constexpr) void apply(T* dest, T* src, usize n)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_constructible<T, T const&>)) {
		algo_::reloc_fallible<T const&>(dest, src, n);
	}
};

template <>
struct reloc_impl<which::throw_move> {
	template <typename T>
	VEG_INLINE static VEG_CPP20(constexpr) void apply(T* dest, T* src, usize n) //
			VEG_NOEXCEPT_IF(false) {
		algo_::reloc_fallible<T&&>(dest, src, n);
	}
};

} // namespace algo_
} // namespace internal

namespace nb {

struct backward_destroy_n {
	VEG_TEMPLATE(
			(typename T, typename Fn),
			requires(!VEG_CONCEPT(void_type<T>)),
			VEG_CPP20(constexpr) void
			operator(),
			(ptr, T*),
			(n, i64),
			(fn, Fn))
	const VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_fn<Fn, void, T*>)) {
		internal::algo_::backward_destroy_n_may_throw(ptr, n, fn);
	}
};

struct relocate_n {
	VEG_TEMPLATE(
			(typename T),
			requires(
					!VEG_CONCEPT(const_type<T>) &&
					(VEG_CONCEPT(movable<T>) || VEG_CONCEPT(copyable<T>))),
			VEG_INLINE VEG_CPP20(constexpr) void
			operator(),
			(dest, T*),
			(src, T*),
			(n, i64))
	const VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_movable<T>) ||
			VEG_CONCEPT(nothrow_constructible<T, T const&>)) {

		namespace impl = internal::algo_;
		impl::reloc_impl<
				cpo::is_trivially_relocatable<T>::value   //
						? impl::which::trivial                //
						: VEG_CONCEPT(nothrow_movable<T>)     //
									? impl::which::nothrow_move     //
									: VEG_CONCEPT(copyable<T>)      //
												? impl::which::copy       //
												: impl::which::throw_move //
				>::apply(dest, src, n);
	}
};

struct uninitialized_move_n {
	VEG_TEMPLATE(
			(typename T),
			requires(VEG_CONCEPT(movable<T>)),
			VEG_INLINE VEG_CPP20(constexpr) void
			operator(),
			(dest, T*),
			(src, T*),
			(n, i64))
	const VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_movable<T>)) {
		internal::algo_::uninit_emplace_n<T&&>(dest, src, n);
	}
};

struct uninitialized_copy_n {
	VEG_TEMPLATE(
			(typename T, int = 0),
			requires(VEG_CONCEPT(trivially_copyable<T>) && VEG_CONCEPT(copyable<T>)),
			VEG_INLINE VEG_CPP20(constexpr) void
			operator(),
			(dest, T*),
			(src, T const*),
			(n, i64))
	const VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_copyable<T>)) {
		abi::internal::veglib_opaque_memmove(
				dest, src, sizeof(T) * static_cast<usize>(n));
	}
	VEG_TEMPLATE(
			(typename T),
			requires(!VEG_CONCEPT(trivially_copyable<T>) && VEG_CONCEPT(copyable<T>)),
			VEG_INLINE VEG_CPP20(constexpr) void
			operator(),
			(dest, T*),
			(src, T const*),
			(n, i64))
	const VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_copyable<T>)) {
		internal::algo_::uninit_emplace_n<T const&>(dest, src, n);
	}
};

struct copy_n {
	VEG_TEMPLATE(
			(typename T),
			requires(VEG_CONCEPT(copy_assignable<T>)),
			VEG_INLINE VEG_CPP20(constexpr) void
			operator(),
			(dest, T*),
			(src, T const*),
			(n, i64))
	const VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_copy_assignable<T>)) {
		for (i64 i = 0; i < n; ++i) {
			dest[i] = src[i];
		}
	}
};

struct mixed_init_copy_n {
	VEG_TEMPLATE(
			(typename T, int = 0),
			requires(
					VEG_CONCEPT(trivially_copyable<T>) &&
					VEG_CONCEPT(copy_assignable<T>) && VEG_CONCEPT(copyable<T>)),
			VEG_INLINE VEG_CPP20(constexpr) void
			operator(),
			(dest, T*),
			(src, T const*),
			(n, i64),
			(n_init, i64))
	const VEG_NOEXCEPT {
		(void)n_init;
		abi::internal::veglib_opaque_memmove(
				dest, src, sizeof(T) * static_cast<usize>(n));
	}

	VEG_TEMPLATE(
			(typename T),
			requires(
					!VEG_CONCEPT(trivially_copyable<T>) &&
					VEG_CONCEPT(copy_assignable<T>) && VEG_CONCEPT(copyable<T>)),
			VEG_INLINE VEG_CPP20(constexpr) void
			operator(),
			(dest, T*),
			(src, T const*),
			(n, i64),
			(n_init, i64))
	const VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_copy_assignable<T>) && VEG_CONCEPT(copyable<T>)) {
		n_init = n < n_init ? n : n_init;
		copy_n{}(dest, src, n_init);
		uninitialized_copy_n{}(dest + n_init, src + n_init, n - n_init);
	}
};

} // namespace nb
VEG_NIEBLOID(backward_destroy_n);
VEG_NIEBLOID(relocate_n);
VEG_NIEBLOID(uninitialized_move_n);
VEG_NIEBLOID(uninitialized_copy_n);
VEG_NIEBLOID(copy_n);
VEG_NIEBLOID(mixed_init_copy_n);
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_ALGORITHM_HPP_PLKQ80IYS */
