#include <cstring>
#include "veg/internal/prologue.hpp"

namespace veg {
namespace mem {
using usize = decltype(sizeof(0));
auto memmove(void* dest, void const* src, usize nbytes) noexcept -> void* {
	// TODO: remove (nbytes > 0) check?
	// - memmove(nullptr, _, 0)
	// - memmove(_, nullptr, 0)
	// formally UB but shouldn't do anything bad
	if (nbytes > 0) {
		std::memmove(dest, src, nbytes);
	}
	return dest;
}
} // namespace mem
} // namespace veg
#include "veg/internal/epilogue.hpp"
