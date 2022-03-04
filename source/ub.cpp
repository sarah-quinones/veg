#include <cstring>
#include "veg/internal/has_asan.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
namespace mem {
using usize = decltype(sizeof(0));

#if VEG_HAS_ASAN
__attribute__((no_sanitize("address")))
#endif
auto memmove(void* dest, void const* src, usize nbytes) noexcept -> void* {
	// - memmove(nullptr, _, 0)
	// - memmove(_, nullptr, 0)
	// formally UB but shouldn't do anything bad
	return std::memmove(dest, src, nbytes);
}
} // namespace mem
} // namespace veg
#include "veg/internal/epilogue.hpp"
