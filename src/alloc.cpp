#define _ISOC99_SOURCE
#define _ISOC11_SOURCE

#include "veg/memory/alloc.hpp"
#include "veg/util/assert.hpp"

#include <cstddef> // std::max_align_t
#include <cstring> // std::{memcpy, memset}
#include <cstdint> // std::uintptr_t
#include <new>     // std::bad_alloc

#include "veg/internal/.external/hedley.ext.h"
#include "veg/internal/prologue.hpp"

#include <cstdlib> // std::{malloc, calloc, realloc, free}

#if defined(__linux__)
#include <malloc.h> // malloc_usable_size
#include <stdlib.h> // aligned_alloc // NOLINT

#define VEG_MSIZE_OR(Fallback, Ptr)                                            \
	((void)(Fallback), ::veg::usize{malloc_usable_size(Ptr)})
#define VEG_ALIGNED_MSIZE_OR(Fallback, Align, Ptr)                             \
	VEG_MSIZE_OR(((void)(Align), (Fallback)), Ptr)
#elif defined(_MSC_VER)
#include <malloc.h> // malloc_usable_size

#define VEG_MSIZE_OR(Fallback, Ptr)                                            \
	((void)(Fallback), ::veg::usize{_msize((Ptr))})
#define VEG_ALIGNED_MSIZE_OR(Fallback, Align Ptr)                              \
	((void)(Fallback), ::veg::usize{_aligned_msize((Ptr), (Align), 0)})
#else
#define VEG_MSIZE_OR(Fallback, Ptr) ((void)(Ptr), ::veg::usize{Fallback})
#define VEG_ALIGNED_MSIZE_OR(Fallback, Align, Ptr)                             \
	((void)(Ptr), (void)(Align), ::veg::usize{Fallback})
#endif

// TODO: macos : ???

namespace veg {
namespace internal {
[[noreturn]] HEDLEY_NEVER_INLINE void throw_bad_alloc() {
	throw std::bad_alloc{};
}
} // namespace internal

namespace internal {
VEG_INLINE constexpr auto uround_up(usize n, usize k) noexcept -> usize {
	return (n + k - 1) / k * k;
}
template <typename T>
VEG_INLINE constexpr auto uround_up_pow2(T n, T k) noexcept -> T {
	return T(n + k - 1) & T(~T(k - 1));
}
VEG_INLINE constexpr auto umin2(usize a, usize b) noexcept -> usize {
	return (a < b) ? a : b;
}
VEG_INLINE constexpr auto umax2(usize a, usize b) noexcept -> usize {
	return (a > b) ? a : b;
}

VEG_INLINE static auto aligned_alloc_fallback_impl(
		usize const align, usize const nbytes, bool zeroed) VEG_ALWAYS_NOEXCEPT
		-> abi::RawAlloc {
	VEG_INTERNAL_ASSERT_PRECONDITIONS(
			(align > alignof(std::max_align_t)), //
			((align & (align - 1)) == 0));

#ifndef VEG_USE_FALLBACK_ALIGNED_ALLOC
#define VEG_USE_FALLBACK_ALIGNED_ALLOC 0
#endif

#if VEG_USE_FALLBACK_ALIGNED_ALLOC

	static_assert(sizeof(usize) <= sizeof(std::uintptr_t), ".");

	usize requested_nbytes =
			nbytes + sizeof(void*) + (align - alignof(std::max_align_t));
	void* const alloc_ptr = zeroed //
	                            ? std::calloc(requested_nbytes, 1)
	                            : std::malloc(requested_nbytes);

	if (alloc_ptr == nullptr) {
		return {nullptr, 0};
	}
	usize const actual_nbytes = VEG_MSIZE_OR(requested_nbytes, alloc_ptr);

	auto uptr = reinterpret_cast<uintptr_t>(alloc_ptr);
	auto uptr_after_header = reinterpret_cast<uintptr_t>(alloc_ptr);
	auto aligned_uptr =
			internal::uround_up_pow2(uptr_after_header, std::uintptr_t{align});
	void* const aligned_ptr = static_cast<mem::byte*>(alloc_ptr) +
	                          usize(aligned_uptr - uptr_after_header);
	void* const header_ptr = static_cast<mem::byte*>(aligned_ptr) - sizeof(void*);
	std::memcpy(header_ptr, &alloc_ptr, sizeof(void*));

	usize const consumed_nbytes =
			static_cast<mem::byte*>(aligned_ptr) - static_cast<mem::byte*>(alloc_ptr);
	usize const leftover_bytes = actual_nbytes - consumed_nbytes;

	if (zeroed) {
		std::memset(
				static_cast<mem::byte*>(aligned_ptr) + requested_nbytes,
				0,
				actual_nbytes - requested_nbytes);
	}
	return {aligned_ptr, leftover_bytes};
#else // do not use fallback

	usize const requested_nbytes = internal::uround_up(nbytes, align);

#if defined(__linux__)
	void* const alloc_ptr = aligned_alloc(align, requested_nbytes);
#elif defined(_MSC_VER)
	void* const alloc_ptr = _aligned_malloc(requested_nbytes, align);
#else // not linux or windows
#error "unsupported platform"
#endif // platform?

	if (alloc_ptr == nullptr) {
		return {nullptr, 0};
	}

	usize const actual_nbytes =
			VEG_ALIGNED_MSIZE_OR(requested_nbytes, align, alloc_ptr);
	if (zeroed) {
		std::memset(alloc_ptr, 0, actual_nbytes);
	}
	return {alloc_ptr, actual_nbytes};

#endif // use fallback?
}

VEG_INLINE
static auto aligned_realloc_fallback_impl(
		usize const align,
		abi::RawAlloc const prev_alloc,
		usize const nbytes,
		bool zero_fresh_alloc) VEG_ALWAYS_NOEXCEPT -> abi::RawAlloc {

#if VEG_USE_FALLBACK_ALIGNED_ALLOC || !defined(_MSC_VER)
	usize const requested_nbytes = internal::uround_up(nbytes, align);

	abi::RawAlloc const alloc = internal::aligned_alloc_fallback_impl(
			align, requested_nbytes, zero_fresh_alloc);
	usize const copy_bytes = internal::umin2(prev_alloc.cap, nbytes);

	if (alloc.data == nullptr) {
		return {nullptr, 0};
	}
	if (prev_alloc.data != nullptr) {
		std::memcpy(alloc.data, prev_alloc.data, copy_bytes);
		abi::mem::veglib_aligned_free(
				align, abi::RawAlloc{prev_alloc.data, prev_alloc.cap});
	}

	return alloc;

#else  // windows and don't use fallback

	usize const requested_nbytes = nbytes;

	void* const alloc_ptr =
			((prev_alloc.data == nullptr) && zero_fresh_alloc)
					? _aligned_recalloc(nullptr, requested_nbytes, 1, align)
					: _aligned_realloc(prev_alloc.data, requested_nbytes, align);
	if (alloc_ptr == nullptr) {
		return {nullptr, 0};
	}
	usize const actual_nbytes =
			VEG_ALIGNED_MSIZE_OR(requested_nbytes, align, alloc_ptr);
	return {alloc_ptr, actual_nbytes};
#endif // use fallback or !windows?
}

VEG_INLINE
static auto aligned_realloc_impl(
		usize const align,
		abi::RawAlloc const prev_alloc,
		usize const nbytes,
		bool const zeroed) VEG_ALWAYS_NOEXCEPT -> abi::RawAlloc {

	static_assert(alignof(void*) <= alignof(std::max_align_t), ".");

	bool const align_fallback = (align > alignof(std::max_align_t));
	usize const copy_bytes = internal::umin2(prev_alloc.cap, nbytes);

	if (align_fallback) {
		abi::RawAlloc non_zeroed = internal::aligned_realloc_fallback_impl(
				align, prev_alloc, nbytes, zeroed);
		if (zeroed) {
			if (prev_alloc.data != nullptr) {
				std::memset(
						static_cast<mem::byte*>(non_zeroed.data) + copy_bytes,
						0,
						non_zeroed.cap - copy_bytes);
			}
		}
		return non_zeroed;
	}

	bool const use_realloc = prev_alloc.data != nullptr;
	usize const requested_nbytes = internal::umax2(nbytes, align);

	void* const alloc_ptr = use_realloc
	                            ? std::realloc(prev_alloc.data, requested_nbytes)
	                            : (zeroed ? std::calloc(requested_nbytes, 1)
	                                      : std::malloc(requested_nbytes));

	if (alloc_ptr == nullptr) {
		return {nullptr, 0};
	}

	usize const actual_nbytes =
			VEG_ALIGNED_MSIZE_OR(requested_nbytes, align, alloc_ptr);
	if (zeroed) {
		usize const ptr_offset =
				use_realloc ? copy_bytes // zero section after copy

										: requested_nbytes; // called calloc
		                                    // zero section after new allocation

		// until end of allocation
		std::memset(
				static_cast<mem::byte*>(alloc_ptr) + ptr_offset,
				0,
				actual_nbytes - ptr_offset);
	}

	return {
			alloc_ptr,
			actual_nbytes,
	};
}

} // namespace internal

namespace abi {
inline namespace VEG_ABI_VERSION {
extern "C" {

namespace mem {
auto veglib_aligned_alloc(
		usize* num_accessible_bytes, usize align, usize nbytes) VEG_ALWAYS_NOEXCEPT
		-> void* {
	auto out = ::veg::internal::aligned_realloc_impl(align, {}, nbytes, false);
	*num_accessible_bytes = out.cap;
	return out.data;
}
auto veglib_aligned_alloc_zeroed(
		usize* num_accessible_bytes, usize align, usize nbytes) VEG_ALWAYS_NOEXCEPT
		-> void* {
	auto out = ::veg::internal::aligned_realloc_impl(align, {}, nbytes, true);
	*num_accessible_bytes = out.cap;
	return out.data;
}
auto veglib_aligned_realloc(
		usize* num_accessible_bytes, usize align, RawAlloc prev_alloc, usize nbytes)
		VEG_ALWAYS_NOEXCEPT -> void* {
	auto out =
			::veg::internal::aligned_realloc_impl(align, prev_alloc, nbytes, false);
	*num_accessible_bytes = out.cap;
	return out.data;
}
auto veglib_aligned_realloc_zeroed(
		usize* num_accessible_bytes, usize align, RawAlloc prev_alloc, usize nbytes)
		VEG_ALWAYS_NOEXCEPT -> void* {
	auto out =
			::veg::internal::aligned_realloc_impl(align, prev_alloc, nbytes, true);
	*num_accessible_bytes = out.cap;
	return out.data;
}

void veglib_aligned_free(usize align, RawAlloc alloc) VEG_ALWAYS_NOEXCEPT {
	(void)align;
#if !VEG_USE_FALLBACK_ALIGNED_ALLOC && defined(__linux__)
	std::free(alloc.data);
#else

	if (align <= alignof(std::max_align_t)) {
		std::free(alloc.data);
	} else {
#if VEG_USE_FALLBACK_ALIGNED_ALLOC
		void* alloc_ptr; // NOLINT
		std::memcpy(
				&alloc_ptr,
				static_cast<byte*>(alloc.data) - sizeof(void*),
				sizeof(void*));
		std::free(alloc_ptr);
#else // do not use fallback

#if defined(__linux__)
#error "already handled"
#elif defined(_MSC_VER)
		_aligned_free(alloc.data);
#else // not linux or windows
#error "unsupported platform"
#endif // platform?
#endif // use fallback?
#endif // linux no fallback?
}
} // namespace mem

} // namespace mem
} // namespace VEG_ABI_VERSION
} // namespace abi
} // namespace veg
#include "veg/internal/epilogue.hpp"
