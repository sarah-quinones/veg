#include <cstring>

namespace veg {
namespace internal {
namespace memory {
using usize = decltype(sizeof(0));
auto opaque_memmove(void* dest, void const* src, usize nbytes) noexcept
    -> void* {
  if (nbytes > 0) {
    std::memmove(dest, src, nbytes);
  }
  return dest;
}
} // namespace memory
} // namespace internal
} // namespace veg
