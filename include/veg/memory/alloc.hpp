#ifndef VEG_ALLOC_HPP_TAWYRUICS
#define VEG_ALLOC_HPP_TAWYRUICS

#include "veg/internal/typedefs.hpp"
#include "veg/internal/macros.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
namespace internal {
[[noreturn]] HEDLEY_NEVER_INLINE void throw_bad_alloc();
} // namespace internal
namespace mem {
using byte = unsigned char;
template <typename T>
struct AllocBlock {
	T* data;
	usize cap;
};
} // namespace mem

namespace abi {
inline namespace VEG_ABI_VERSION {
extern "C" {
struct RawAlloc {
	void* data;
	usize cap;
};
namespace internal {
auto veglib_opaque_memmove(void* dest, void const* src, usize nbytes) noexcept
		-> void*;
} // namespace internal

namespace mem {
using ::veg::mem::byte;

#if defined(HEDLEY_GNUC_VERSION)
#define VEG_ATTRIBUTE_MALLOC __attribute__((__malloc__))
#elif defined(HEDLEY_MSVC_VERSION)
#define VEG_ATTRIBUTE_MALLOC __declspec(restrict)
#endif

void veglib_aligned_free(usize align, RawAlloc alloc) VEG_ALWAYS_NOEXCEPT;
VEG_ATTRIBUTE_MALLOC auto veglib_aligned_alloc(
		usize* num_accessible_bytes, usize align, usize nbytes) VEG_ALWAYS_NOEXCEPT
		-> void*;
VEG_ATTRIBUTE_MALLOC auto veglib_aligned_alloc_zeroed(
		usize* num_accessible_bytes, usize align, usize nbytes) VEG_ALWAYS_NOEXCEPT
		-> void*;
auto veglib_aligned_realloc(
		usize* num_accessible_bytes, usize align, RawAlloc prev_alloc, usize nbytes)
		VEG_ALWAYS_NOEXCEPT -> void*;
auto veglib_aligned_realloc_zeroed(
		usize* num_accessible_bytes, usize align, RawAlloc prev_alloc, usize nbytes)
		VEG_ALWAYS_NOEXCEPT -> void*;
} // namespace mem
} // extern "C"
} // namespace VEG_ABI_VERSION
} // namespace abi

namespace mem {
struct SystemAlloc {};
} // namespace mem
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_ALLOC_HPP_TAWYRUICS */
