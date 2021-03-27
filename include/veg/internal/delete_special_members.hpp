#ifndef VEG_DELETE_SPECIAL_MEMBERS_HPP_2HKRCVWUS
#define VEG_DELETE_SPECIAL_MEMBERS_HPP_2HKRCVWUS

#include "veg/internal/prologue.hpp"
#include "veg/internal/typedefs.hpp"

namespace veg {
inline namespace VEG_ABI {
namespace internal {
struct nodefault_ctor {
	nodefault_ctor() = delete;
};
struct nomove_assign {
	nomove_assign() = default;
	~nomove_assign() = default;
	nomove_assign(nomove_assign const&) = default;
	nomove_assign(nomove_assign&&) noexcept = default;
	auto operator=(nomove_assign const&) -> nomove_assign& = default;
	auto operator=(nomove_assign&&) noexcept -> nomove_assign& = delete;
};
struct nocopy_assign {
	nocopy_assign() = default;
	~nocopy_assign() = default;
	nocopy_assign(nocopy_assign const&) = default;
	nocopy_assign(nocopy_assign&&) noexcept = default;
	auto operator=(nocopy_assign const&) -> nocopy_assign& = delete;
	auto operator=(nocopy_assign&&) noexcept -> nocopy_assign& = default;
};
struct nomove_ctor {
	nomove_ctor() = default;
	~nomove_ctor() = default;
	nomove_ctor(nomove_ctor const&) = default;
	nomove_ctor(nomove_ctor&&) noexcept = delete;
	auto operator=(nomove_ctor const&) -> nomove_ctor& = default;
	auto operator=(nomove_ctor&&) noexcept -> nomove_ctor& = default;
};
struct nocopy_ctor {
	nocopy_ctor() = default;
	~nocopy_ctor() = default;
	nocopy_ctor(nocopy_ctor const&) = delete;
	nocopy_ctor(nocopy_ctor&&) noexcept = default;
	auto operator=(nocopy_ctor const&) -> nocopy_ctor& = default;
	auto operator=(nocopy_ctor&&) noexcept -> nocopy_ctor& = default;
};

} // namespace internal
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_DELETE_SPECIAL_MEMBERS_HPP_2HKRCVWUS */
