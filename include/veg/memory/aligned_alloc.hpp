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
auto aligned_alloc(usize align, i64 nbytes) VEG_NOEXCEPT_IF(false) -> void*;
void aligned_free(void* ptr, usize align, i64 nbytes) VEG_ALWAYS_NOEXCEPT;
} // namespace mem
} // namespace VEG_ABI_VERSION
} // namespace abi
namespace mem {
namespace nb {
struct aligned_alloc {
	auto operator()(usize align, i64 nbytes) const VEG_NOEXCEPT_IF(false)
			-> void* {
		return abi::mem::aligned_alloc(align, nbytes);
	}
};
struct aligned_free {
	void
	operator()(void* ptr, usize align, i64 nbytes) const VEG_ALWAYS_NOEXCEPT {
		if (ptr != nullptr) {
			abi::mem::aligned_free(ptr, align, nbytes);
		}
	}
};
} // namespace nb
VEG_NIEBLOID(aligned_alloc);
VEG_NIEBLOID(aligned_free);
} // namespace mem
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_ALIGNED_ALLOC_HPP_AZIYTA7QS */
