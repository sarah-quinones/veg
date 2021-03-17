#include "veg/internal/memory.hpp"
#include "veg/internal/narrow.hpp"
#include <cstdlib>
#include <cstddef>
#include <new>

namespace veg {
namespace internal {
namespace memory {

constexpr auto narrow = fn::narrow<usize>{};

[[noreturn]] HEDLEY_NEVER_INLINE void throw_bad_alloc() {
  throw std::bad_alloc{};
}

auto aligned_alloc(i64 align, i64 nbytes) -> void* {
  void* p = ::aligned_alloc(narrow(align), narrow(nbytes));
  if (p == nullptr) {
    throw_bad_alloc();
  }
  return p;
}

auto aligned_realloc(void* ptr, i64 align, i64 nbytes_old, i64 nbytes_new)
    -> void* {
  if (align <= alignof(std::max_align_t)) {
    void* p = std::realloc(ptr, narrow(nbytes_new));
    if (p == nullptr) {
      throw_bad_alloc();
    }
    return p;
  }

  void* out = memory::aligned_alloc(nbytes_new, align);
  memory::aligned_free(ptr, nbytes_old);
  return out;
}

void aligned_free(void* ptr, i64 nbytes) noexcept {
  (void)nbytes;
  std::free(ptr);
}

} // namespace memory
} // namespace internal
} // namespace veg
