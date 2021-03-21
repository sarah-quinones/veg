#include "veg/internal/memory.hpp"
#include <cstdlib>
#include <cstddef>
#include <new>
#include "veg/internal/prologue.hpp"

namespace veg {
namespace abi {
inline namespace VEG_ABI_VERSION {
namespace internal {
[[noreturn]] HEDLEY_NEVER_INLINE void throw_bad_alloc() {
	throw std::bad_alloc{};
}

} // namespace internal

auto aligned_alloc(i64 align, i64 nbytes) -> void* {
	void* p = ::aligned_alloc(usize(align), usize(nbytes));
	if (p == nullptr) {
		abi::internal::throw_bad_alloc();
	}
	return p;
}

auto aligned_realloc(void* ptr, i64 align, i64 nbytes_old, i64 nbytes_new)
		-> void* {
	if (align <= alignof(std::max_align_t)) {
		void* p = std::realloc(ptr, usize(nbytes_new));
		if (p == nullptr) {
			abi::internal::throw_bad_alloc();
		}
		return p;
	}

	void* out = abi::aligned_alloc(nbytes_new, align);
	abi::aligned_free(ptr, nbytes_old);
	return out;
}

void aligned_free(void* ptr, i64 nbytes) noexcept {
	(void)nbytes;
	std::free(ptr);
}

} // namespace VEG_ABI_VERSION
} // namespace abi
} // namespace veg
#include "veg/internal/epilogue.hpp"
