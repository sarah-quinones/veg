#ifndef VEG_ALIGNED_ALLOC_HPP_AZIYTA7QS
#define VEG_ALIGNED_ALLOC_HPP_AZIYTA7QS

#include "veg/internal/typedefs.hpp"
#include "veg/internal/macros.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
namespace abi {
inline namespace VEG_ABI_VERSION {
namespace internal {
auto opaque_memmove(void* dest, void const* src, usize nbytes) noexcept
		-> void*;
} // namespace internal

namespace mem {
auto aligned_alloc(i64 align, i64 nbytes) noexcept(false) -> void*;
void aligned_free(void* ptr, i64 nbytes) noexcept;
} // namespace mem
} // namespace VEG_ABI_VERSION
} // namespace abi
inline namespace VEG_ABI {
namespace mem {
namespace nb {
struct aligned_alloc {
	auto operator()(i64 align, i64 nbytes) const noexcept(false) -> void* {
		return abi::mem::aligned_alloc(align, nbytes);
	}
};
struct aligned_free {
	void operator()(void* ptr, i64 nbytes) const noexcept {
		if (ptr != nullptr) {
			abi::mem::aligned_free(ptr, nbytes);
		}
	}
};
} // namespace nb
VEG_NIEBLOID(aligned_alloc);
VEG_NIEBLOID(aligned_free);
} // namespace mem
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_ALIGNED_ALLOC_HPP_AZIYTA7QS */
