#ifndef VEG_TYPE_TRAITS_HPP_Z3FBQSJ2S
#define VEG_TYPE_TRAITS_HPP_Z3FBQSJ2S

#include "veg/internal/std.hpp"
#include "veg/internal/typedefs.hpp"
#include "veg/internal/macros.hpp"
#include "veg/type_traits/core.hpp"
#include "veg/type_traits/constructible.hpp"
#include "veg/type_traits/assignable.hpp"
#include "veg/type_traits/invocable.hpp"
#include "veg/type_traits/primitives.hpp"
#include "veg/type_traits/value_category.hpp"
#include "veg/internal/delete_special_members.hpp"
#include "veg/util/swap.hpp"
#include "veg/util/get.hpp"
#include "veg/util/defer.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
inline namespace VEG_ABI {
// dev-only
namespace meta {

template <typename... Ts>
VEG_CPP14(constexpr)
void unused(Ts const&... /*unused*/) {}

template <typename T>
constexpr auto max2(T a, T b) noexcept -> T {
	return a > b ? a : b;
}

template <typename T>
constexpr auto max_of_slice(T const* arr, i64 size) noexcept -> T {
	return size == 1 ? arr[0]
									 : meta::max2(arr[0], max_of_slice(arr + 1, size - 1));
}

template <typename T>
constexpr auto max_of(std::initializer_list<T> lst) noexcept -> T {
	return meta::max_of_slice(lst.begin(), static_cast<i64>(lst.size()));
}
} // namespace meta

using meta::uncvref_t;
using meta::unref_t;

} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_TYPE_TRAITS_HPP_Z3FBQSJ2S */
