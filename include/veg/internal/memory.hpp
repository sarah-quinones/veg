#ifndef __VEG_NEW_HPP_43XG2FSKS
#define __VEG_NEW_HPP_43XG2FSKS

#include "veg/internal/type_traits.hpp"
#include "veg/internal/std.hpp"

#if __VEG_HAS_BUILTIN(__builtin_launder) || __GNUC__ >= 7
#define VEG_LAUNDER(p) __builtin_launder(p)
#elif __cplusplus >= 201703L
#define VEG_LAUNDER(p) ::std::launder(p)
#else
#define VEG_LAUNDER(p) (+p)
#endif

namespace veg {
namespace internal {
namespace memory {
auto opaque_memmove(void* dest, void const* src, usize nbytes) noexcept
		-> void*;

auto aligned_alloc(i64 align, i64 nbytes) noexcept(false) -> void*;
void aligned_free(void* ptr, i64 nbytes) noexcept;
} // namespace memory
} // namespace internal

namespace fn {
struct aligned_alloc {
	auto operator()(i64 align, i64 nbytes) const noexcept(false) -> void* {
		return internal::memory::aligned_alloc(align, nbytes);
	}
};
struct aligned_free {
	void operator()(void* ptr, i64 nbytes) const noexcept {
		if (ptr != nullptr) {
			internal::memory::aligned_free(ptr, nbytes);
		}
	}
};
} // namespace fn
__VEG_ODR_VAR(aligned_alloc, fn::aligned_alloc);
__VEG_ODR_VAR(aligned_free, fn::aligned_free);

#if !(__VEG_HAS_BUILTIN(__builtin_addressof) || __cplusplus >= 201703L)

namespace internal {
namespace memory {
struct member_addr {
	template <typename T>
	using type = decltype(void(__VEG_DECLVAL(T&).operator&()));

	template <typename T>
	HEDLEY_ALWAYS_INLINE static auto apply(T& var) noexcept -> T* {
		using char_ref = char&;
		return static_cast<T*>(static_cast<void*>(&char_ref(var)));
	}
};
struct adl_addr : member_addr {
	template <typename T>
	using type = decltype(void(operator&(__VEG_DECLVAL(T&))));
};
struct builtin_addr : std::true_type {
	template <typename T>
	HEDLEY_ALWAYS_INLINE static constexpr auto apply(T& var) noexcept -> T* {
		return &var;
	}
};
template <typename T>
struct has_member_addr : meta::is_detected<member_addr::type, T&>,
												 member_addr {};
template <typename T>
struct has_adl_addr : meta::is_detected<adl_addr::type, T&>, adl_addr {};

template <typename T>
struct addr_impl
		: meta::disjunction<has_member_addr<T>, has_adl_addr<T>, builtin_addr> {};

} // namespace memory
} // namespace internal

#endif

namespace internal {
namespace memory {

struct uniform_init_ctor {
	template <typename T, typename... Args>
	HEDLEY_ALWAYS_INLINE static auto apply(T* mem, Args&&... args) noexcept(
			noexcept(new (mem) T{VEG_FWD(args)...})) -> T* {
		return new (mem) T{VEG_FWD(args)...};
	}
};
struct ctor {
	template <typename T, typename... Args>
	HEDLEY_ALWAYS_INLINE static __VEG_CPP20(constexpr) auto apply(
			T* mem, Args&&... args) noexcept(__VEG_CONCEPT(meta::
	                                                       nothrow_constructible<
																														 T,
																														 Args&&...>))
			-> T* {
#if __cplusplus >= 202002L
		return ::std::construct_at(mem, VEG_FWD(args)...);
#else
		return new (mem) T(VEG_FWD(args)...);
#endif
	}
};

template <typename T, typename... Args>
struct uniform_constructible
		: meta::bool_constant<__VEG_CONCEPT(
					meta::uniform_init_constructible<T, Args&&...>)>,
			uniform_init_ctor {};
template <typename T, typename... Args>
struct constructible
		: meta::bool_constant<__VEG_CONCEPT(meta::constructible<T, Args&&...>)>,
			ctor {};

template <typename T, typename... Args>
struct ctor_at_impl : meta::disjunction<
													constructible<T, Args&&...>,
													uniform_constructible<T, Args&&...>> {};

template <typename Fn>
struct fn_to_convertible {
	Fn&& fn;
	HEDLEY_ALWAYS_INLINE constexpr explicit
	operator meta::invoke_result_t<Fn&&>() const
			noexcept(__VEG_CONCEPT(meta::nothrow_invocable<Fn&&>)) {
		return VEG_FWD(fn)();
	}
};
__VEG_CPP17(

		template <typename Fn> fn_to_convertible(Fn&&) -> fn_to_convertible<Fn&&>;

)
} // namespace memory
} // namespace internal

namespace fn {
struct construct_at {
	VEG_TEMPLATE(
			(typename T, typename... Args),
			requires(
					!__VEG_CONCEPT(meta::const_<T>) &&
					__VEG_CONCEPT(__VEG_DISJUNCTION(
							(meta::constructible<T, Args&&...>),
							(meta::uniform_init_constructible<T, Args&&...>)))),
			HEDLEY_ALWAYS_INLINE __VEG_CPP20(constexpr) auto
			operator(),
			(mem, T*),
			(... args, Args&&))
	const noexcept(__VEG_CONCEPT(meta::nothrow_constructible<T, Args&&...>))->T* {
		return internal::memory::ctor_at_impl<T, Args&&...>::apply(
				mem, VEG_FWD(args)...);
	}
};

struct construct_with {
	VEG_TEMPLATE(
			(typename T, typename Fn, typename... Args),
			requires(
					!__VEG_CONCEPT(meta::const_<T>) &&
					__VEG_CONCEPT(meta::invocable<Fn&&, Args&&...>) &&
					__VEG_SAME_AS(T, (meta::invoke_result_t<Fn&&, Args&&...>))),
			HEDLEY_ALWAYS_INLINE __VEG_CPP20(constexpr) auto
			operator(),
			(mem, T*),
			(fn, Fn&&),
			(... args, Args&&))
	const noexcept(__VEG_CONCEPT(meta::nothrow_invocable<Fn&&, Args&&...>))->T* {
#if __cplusplus >= 202002L

		return ::std::construct_at(
				mem,
				internal::memory::fn_to_convertible{
						[&]() noexcept(
								__VEG_CONCEPT(meta::nothrow_invocable<Fn&&, Args&&...>))
								-> meta::invoke_result_t<Fn&&, Args&&...> {
							return invoke{}(VEG_FWD(fn), VEG_FWD(args)...);
						}}

		);
#else
		return new (mem) T(invoke{}(VEG_FWD(fn), VEG_FWD(args)...));
#endif
	}
};

struct destroy_at {
	VEG_TEMPLATE(
			(typename T, typename... Args),
			requires !__VEG_CONCEPT(meta::void_<T>),
			HEDLEY_ALWAYS_INLINE __VEG_CPP20(constexpr) void
			operator(),
			(mem, T*))
	const noexcept { mem->~T(); }
};

struct addressof {
	template <typename T>
	constexpr void operator()(T const&& var) const noexcept = delete;

	template <typename T>
	HEDLEY_ALWAYS_INLINE constexpr auto operator()(T& var) const noexcept -> T* {
#if __VEG_HAS_BUILTIN(__builtin_addressof)

		return __builtin_addressof(var);

#elif __cplusplus >= 201703L

		return ::std::addressof(var);

#else

		return internal::memory::addr_impl<T>::apply(var);

#endif
	}
};
} // namespace fn

__VEG_ODR_VAR(construct_at, fn::construct_at);
__VEG_ODR_VAR(construct_with, fn::construct_with);
__VEG_ODR_VAR(destroy_at, fn::destroy_at);
__VEG_ODR_VAR(addressof, fn::addressof);

} // namespace veg

#endif /* end of include guard __VEG_NEW_HPP_43XG2FSKS */
