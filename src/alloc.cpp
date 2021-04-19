#include "veg/memory/aligned_alloc.hpp"
#include <cstdlib>
#include <cstddef>
#include <new>
#include "veg/internal/.external/hedley.ext.h"
#include "veg/internal/prologue.hpp"

namespace veg {
namespace abi {
inline namespace VEG_ABI_VERSION {
namespace internal {
[[noreturn]] HEDLEY_NEVER_INLINE void throw_bad_alloc() {
	throw std::bad_alloc{};
}

} // namespace internal

namespace mem {
auto aligned_alloc(usize align, i64 nbytes) -> void* {
	void* p = ::aligned_alloc(usize(align), usize(nbytes));
	if (p == nullptr) {
		abi::internal::throw_bad_alloc();
	}
	return p;
}

void aligned_free(void* ptr, usize align, i64 nbytes) noexcept {
	(void)nbytes, (void)align;
	std::free(ptr);
}
} // namespace mem
} // namespace VEG_ABI_VERSION
} // namespace abi
} // namespace veg
#include "veg/internal/epilogue.hpp"
