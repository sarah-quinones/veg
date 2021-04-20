#ifndef VEG_PLACEMENT_HPP_LP0HMLTPS
#define VEG_PLACEMENT_HPP_LP0HMLTPS

#include "veg/type_traits/constructible.hpp"
#include "veg/internal/std.hpp"
#include "veg/internal/tuple_generic.hpp"
#include "veg/type_traits/invocable.hpp"
#include "veg/internal/prologue.hpp"
#include <new>

// construct_at
#if __cplusplus >= 202002L

// STD INDCLUDE: construct_at
#if __VEG_HAS_INCLUDE(<bits/stl_construct.h>) &&                                              \
		__VEG_HAS_INCLUDE(<bits/stl_iterator_base_types.h>) &&                                \
				__VEG_HAS_INCLUDE(<bits/stl_iterator_base_funcs.h>)
#include <bits/stl_iterator_base_types.h>
#include <bits/stl_iterator_base_funcs.h>
#include <bits/stl_construct.h>
#else
#include <memory>
#endif

#endif

#if VEG_HAS_BUILTIN(__builtin_launder) || __GNUC__ >= 7
#define VEG_LAUNDER(p) (__builtin_launder(p))
#elif __cplusplus >= 201703L
#include <new>
#define VEG_LAUNDER(p) (::std::launder(p))
#else
#define VEG_LAUNDER(p) (+p)
#endif

namespace veg {
inline namespace VEG_ABI {

namespace internal {
namespace mem_ {

template <typename Fn, typename... Args>
struct FromCallable {
	Fn&& _fn;
	internal::SimpleTuple<Args&&...> _args;

	HEDLEY_ALWAYS_INLINE constexpr operator meta::invoke_result_t<Fn, Args&&...>()
			const&& noexcept(VEG_CONCEPT(nothrow_invocable<Fn>)) {
		return internal::tup_::unpack_args_impl(
				meta::type_sequence<Args&&...>{}, VEG_FWD(_fn), _args);
	}
};
} // namespace mem_
} // namespace internal

namespace nb {
struct from_callable {
	VEG_TEMPLATE(
			(typename Fn, typename... Args),
			requires(VEG_CONCEPT(invocable<Fn, Args&&...>)),
			constexpr auto
			operator(),
			(fn, Fn&&),
			(... args, Args&&))
	const noexcept->internal::mem_::FromCallable<Fn&&, Args&&...> {
		return {VEG_FWD(fn), {Cvt{}, VEG_FWD(args)...}};
	}
};
} // namespace nb
VEG_NIEBLOID(from_callable);

namespace mem {
namespace nb {
struct launder {
	VEG_TEMPLATE(
			typename T,
			requires(VEG_CONCEPT(complete<T>)),
			HEDLEY_ALWAYS_INLINE constexpr auto
			operator(),
			(mem, T*))
	const noexcept->T* { return VEG_LAUNDER(mem); }
};
#undef VEG_LAUNDER

struct construct_at {
	VEG_TEMPLATE(
			(typename T, typename... Args),
			requires(
					!VEG_CONCEPT(const_type<T>) &&
					VEG_CONCEPT(constructible<T, Args...>)),
			HEDLEY_ALWAYS_INLINE VEG_CPP20(constexpr) auto
			operator(),
			(mem, T*),
			(... args, Args&&))
	const noexcept(VEG_CONCEPT(nothrow_constructible<T, Args...>))->T* {
#if __cplusplus >= 202002L
		return ::std::construct_at(mem, VEG_FWD(args)...);
#else
		return ::new (mem) T(VEG_FWD(args)...);
#endif
	}
};

struct construct_with {
	VEG_TEMPLATE(
			(typename T, typename Fn, typename... Args),
			requires(
					!VEG_CONCEPT(const_type<T>) && VEG_CONCEPT(invocable<Fn, Args...>) &&
					VEG_CONCEPT(same<T, meta::invoke_result_t<Fn, Args...>>)),
			HEDLEY_ALWAYS_INLINE VEG_CPP20(constexpr) auto
			operator(),
			(mem, T*),
			(fn, Fn&&),
			(... args, Args&&))
	const noexcept(VEG_CONCEPT(nothrow_invocable<Fn, Args...>))->T* {
#if __cplusplus >= 202002L

		return ::std::construct_at(
				mem,
				internal::mem_::FromCallable<Fn&&>{
						[&]() noexcept(VEG_CONCEPT(nothrow_invocable<Fn, Args...>)) -> T {
							return VEG_FWD(fn)(VEG_FWD(args)...);
						}}

		);
#else
		return ::new (mem) T(VEG_FWD(fn)(VEG_FWD(args)...));
#endif
	}
};

struct destroy_at {
	VEG_TEMPLATE(
			(typename T, typename... Args),
			requires(!VEG_CONCEPT(void_type<T>)),
			HEDLEY_ALWAYS_INLINE VEG_CPP20(constexpr) void
			operator(),
			(mem, T*))
	const noexcept { mem->~T(); }
};
struct align_next {
	auto operator()(usize alignment, void* ptr) const noexcept -> void* {
		using byte_ptr = unsigned char*;
		std::uintptr_t lo_mask = alignment - 1U;
		std::uintptr_t hi_mask = ~lo_mask;
		auto const intptr = reinterpret_cast<std::uintptr_t>(ptr);
		auto* const byteptr = static_cast<byte_ptr>(ptr);
		auto offset = ((intptr + alignment - 1U) & hi_mask) - intptr;

		return byteptr + offset;
	}
	auto operator()(usize alignment, void const* ptr) const noexcept
			-> void const* {
		return this->operator()(alignment, const_cast<void*>(ptr));
	}
};
struct align_prev {
	auto operator()(usize alignment, void* ptr) const noexcept -> void* {
		using byte_ptr = unsigned char*;
		std::uintptr_t lo_mask = alignment - 1U;
		std::uintptr_t hi_mask = ~lo_mask;
		auto const intptr = reinterpret_cast<std::uintptr_t>(ptr);
		auto* const byteptr = static_cast<byte_ptr>(ptr);
		auto offset = ((intptr)&hi_mask) - intptr;

		return byteptr + offset;
	}
	auto operator()(usize alignment, void const* ptr) const noexcept
			-> void const* {
		return this->operator()(alignment, const_cast<void*>(ptr));
	}
};
} // namespace nb
VEG_NIEBLOID(align_next);
VEG_NIEBLOID(align_prev);
VEG_NIEBLOID(launder);
VEG_NIEBLOID(construct_at);
VEG_NIEBLOID(construct_with);
VEG_NIEBLOID(destroy_at);
} // namespace mem

} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_PLACEMENT_HPP_LP0HMLTPS */
