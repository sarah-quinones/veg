#include <cstring>

namespace veg {
namespace abi {
inline namespace VEG_ABI_VERSION {
namespace internal {
using usize = decltype(sizeof(0));
auto opaque_memmove(void* dest, void const* src, usize nbytes) noexcept
		-> void* {
	if (nbytes > 0) {
		std::memmove(dest, src, nbytes);
	}
	return dest;
}
} // namespace internal
} // namespace VEG_ABI_VERSION
} // namespace abi
} // namespace veg
