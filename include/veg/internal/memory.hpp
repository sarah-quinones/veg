#ifndef __VEG_NEW_HPP_43XG2FSKS
#define __VEG_NEW_HPP_43XG2FSKS

#include "veg/internal/type_traits.hpp"
#include "veg/internal/std.hpp"
#include "veg/internal/prologue.hpp"

#if VEG_HAS_BUILTIN(__builtin_launder) || __GNUC__ >= 7
#define VEG_LAUNDER(p) __builtin_launder(p)
#elif __cplusplus >= 201703L
#define VEG_LAUNDER(p) ::std::launder(p)
#else
#define VEG_LAUNDER(p) (+p)
#endif

namespace veg {
namespace abi {
inline namespace VEG_ABI_VERSION {
namespace internal {
auto opaque_memmove(void* dest, void const* src, usize nbytes) noexcept
		-> void*;
} // namespace internal

namespace mem {
auto aligned_alloc(i64 align, i64 nbytes) noexcept(false) -> void*;
void aligned_free(void* ptr, i64 nbytes) noexcept;
} // namespace mem
} // namespace VEG_ABI_VERSION
} // namespace abi

inline namespace VEG_ABI {

#if !(VEG_HAS_BUILTIN(__builtin_addressof) || __cplusplus >= 201703L)

namespace internal {
namespace memory {
struct member_addr {
	template <typename T>
	using type = decltype(void(VEG_DECLVAL(T&).operator&()));

	template <typename T>
	HEDLEY_ALWAYS_INLINE static auto apply(T& var) noexcept -> T* {
		using char_ref = char&;
		return static_cast<T*>(static_cast<void*>(&char_ref(var)));
	}
};
struct adl_addr : member_addr {
	template <typename T>
	using type = decltype(void(operator&(VEG_DECLVAL(T&))));
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

template <typename Fn>
struct fn_to_convertible {
	Fn&& fn;
	HEDLEY_ALWAYS_INLINE constexpr explicit
	operator meta::invoke_result_t<Fn>() const
			noexcept(VEG_CONCEPT(nothrow_invocable<Fn>)) {
		return VEG_FWD(fn)();
	}
};
VEG_CPP17(

		template <typename Fn> fn_to_convertible(Fn&&) -> fn_to_convertible<Fn&&>;

)
} // namespace memory
} // namespace internal

namespace mem {
namespace niebloid {
struct aligned_alloc {
	auto operator()(i64 align, i64 nbytes) const noexcept(false) -> void* {
		return abi::mem::aligned_alloc(align, nbytes);
	}
};
struct aligned_free {
	void operator()(void* ptr, i64 nbytes) const noexcept {
		if (ptr != nullptr) {
			abi::mem::aligned_free(ptr, nbytes);
		}
	}
};

struct aggregate_construct_at {
	VEG_TEMPLATE(
			(typename T, typename... Args),
			requires(
					!VEG_CONCEPT(const_type<T>) &&
					VEG_CONCEPT(brace_constructible<T, Args...>)),
			HEDLEY_ALWAYS_INLINE VEG_CPP20(constexpr) auto
			operator(),
			(mem, T*),
			(... args, Args&&))
	const noexcept(VEG_CONCEPT(nothrow_brace_constructible<T, Args...>))->T* {
#if __cplusplus >= 202002L
		return ::std::construct_at(
				mem,
				internal::memory::fn_to_convertible{
						[&]() noexcept(VEG_CONCEPT(nothrow_brace_constructible<T, Args...>))
								-> T { return T{VEG_FWD(args)...}; }});
#else
		return new (mem) T{VEG_FWD(args)...};
#endif
	}
};

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
		return new (mem) T(VEG_FWD(args)...);
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
				internal::memory::fn_to_convertible{
						[&]() noexcept(VEG_CONCEPT(nothrow_invocable<Fn, Args...>)) -> T {
							return invoke(VEG_FWD(fn), VEG_FWD(args)...);
						}}

		);
#else
		return new (mem) T(invoke(VEG_FWD(fn), VEG_FWD(args)...));
#endif
	}
};

struct destroy_at {
	VEG_TEMPLATE(
			(typename T, typename... Args),
			requires !VEG_CONCEPT(void_type<T>),
			HEDLEY_ALWAYS_INLINE VEG_CPP20(constexpr) void
			operator(),
			(mem, T*))
	const noexcept { mem->~T(); }
};

struct addressof {
	template <typename T>
	constexpr void operator()(T const&& var) const noexcept = delete;

	template <typename T>
	HEDLEY_ALWAYS_INLINE constexpr auto operator()(T& var) const noexcept -> T* {
#if VEG_HAS_BUILTIN(__builtin_addressof)

		return __builtin_addressof(var);

#elif __cplusplus >= 201703L

		return ::std::addressof(var);

#else

		return internal::memory::addr_impl<T>::apply(var);

#endif
	}
};
} // namespace niebloid
VEG_NIEBLOID(aligned_alloc);
VEG_NIEBLOID(aligned_free);
VEG_NIEBLOID(aggregate_construct_at);
VEG_NIEBLOID(construct_at);
VEG_NIEBLOID(construct_with);
VEG_NIEBLOID(destroy_at);
VEG_NIEBLOID(addressof);
} // namespace mem
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard __VEG_NEW_HPP_43XG2FSKS */
