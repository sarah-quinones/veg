#ifndef VEG_DELETE_SPECIAL_MEMBERS_HPP_2HKRCVWUS
#define VEG_DELETE_SPECIAL_MEMBERS_HPP_2HKRCVWUS

#include "veg/internal/prologue.hpp"
#include "veg/internal/typedefs.hpp"

namespace veg {
inline namespace VEG_ABI {
namespace internal {
struct NoDefaultCtor {
	NoDefaultCtor() = delete;
};
struct NoMoveAssign {
	NoMoveAssign() = default;
	~NoMoveAssign() = default;
	NoMoveAssign(NoMoveAssign const&) = default;
	NoMoveAssign(NoMoveAssign&&) noexcept = default;
	auto operator=(NoMoveAssign const&) -> NoMoveAssign& = default;
	auto operator=(NoMoveAssign&&) noexcept -> NoMoveAssign& = delete;
};
struct NoCopyAssign {
	NoCopyAssign() = default;
	~NoCopyAssign() = default;
	NoCopyAssign(NoCopyAssign const&) = default;
	NoCopyAssign(NoCopyAssign&&) noexcept = default;
	auto operator=(NoCopyAssign const&) -> NoCopyAssign& = delete;
	auto operator=(NoCopyAssign&&) noexcept -> NoCopyAssign& = default;
};
struct NoMoveCtor {
	NoMoveCtor() = default;
	~NoMoveCtor() = default;
	NoMoveCtor(NoMoveCtor const&) = default;
	NoMoveCtor(NoMoveCtor&&) noexcept = delete;
	auto operator=(NoMoveCtor const&) -> NoMoveCtor& = default;
	auto operator=(NoMoveCtor&&) noexcept -> NoMoveCtor& = default;
};
struct NoCopyCtor {
	NoCopyCtor() = default;
	~NoCopyCtor() = default;
	NoCopyCtor(NoCopyCtor const&) = delete;
	NoCopyCtor(NoCopyCtor&&) noexcept = default;
	auto operator=(NoCopyCtor const&) -> NoCopyCtor& = default;
	auto operator=(NoCopyCtor&&) noexcept -> NoCopyCtor& = default;
};

} // namespace internal
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_DELETE_SPECIAL_MEMBERS_HPP_2HKRCVWUS */
