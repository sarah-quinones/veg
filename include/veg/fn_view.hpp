#ifndef __VEG_FN_REF_HPP_IDVWA3EYS
#define __VEG_FN_REF_HPP_IDVWA3EYS

#include "veg/assert.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
namespace abi {
inline namespace VEG_ABI_VERSION {
namespace fn {
template <typename T>
struct nothrow;

union state_ptr {
	void* ptr;
	void (*fn)();
};

template <typename T>
struct raw_parts;

template <typename Ret, typename... Args>
struct raw_parts<Ret(Args...)> {
	state_ptr state;
	auto (*fn_ptr)(state_ptr, Args...) -> Ret;
};

template <typename Ret, typename... Args>
struct raw_parts<nothrow<Ret(Args...)>> {
	state_ptr state;
	auto (*fn_ptr)(state_ptr, Args...) noexcept -> Ret;
};
} // namespace fn
} // namespace VEG_ABI_VERSION
} // namespace abi

inline namespace VEG_ABI {
namespace fn {
using namespace abi::fn;

template <typename T>
struct fn_view;
template <typename T>
struct once_fn_view;

namespace internal {
namespace fnref {

template <typename T>
struct is_fn_ptr : std::false_type {};
template <typename Ret, typename... Args>
struct is_fn_ptr<Ret (*)(Args...)> : std::true_type {};

template <typename T>
using unary_plus = decltype(+VEG_DECLVAL(T &&));

template <typename T>
struct is_convertible_to_fn_ptr_with_unary_plus
		: is_fn_ptr<meta::detected_t<unary_plus, T>> {};

template <typename Ret>
struct discard_void {
	// non void case
	template <typename Fn, typename... Args>
	HEDLEY_ALWAYS_INLINE static auto apply(Fn&& fn, Args&&... args) noexcept(
			VEG_CONCEPT(nothrow_invocable<Fn&&, Args&&...>)) -> Ret {
		return invoke(VEG_FWD(fn), VEG_FWD(args)...);
	}
};
template <>
struct discard_void<void> {
	template <typename Fn, typename... Args>
	HEDLEY_ALWAYS_INLINE static auto apply(Fn&& fn, Args&&... args) noexcept(
			VEG_CONCEPT(nothrow_invocable<Fn&&, Args&&...>)) -> void {
		void(invoke(VEG_FWD(fn), VEG_FWD(args)...));
	}
};

enum struct fn_kind_e {
	fn_ptr,
	mem_fn_ptr,
	fn_obj,
};

template <typename T>
struct fn_kind : std::integral_constant<
										 fn_kind_e,
										 is_convertible_to_fn_ptr_with_unary_plus<T>::value
												 ? fn_kind_e::fn_ptr
												 : fn_kind_e::fn_obj> {};

template <typename Ret, typename... Args>
struct fn_kind<Ret (*)(Args...)>
		: std::integral_constant<fn_kind_e, fn_kind_e::fn_ptr> {};

VEG_CPP17(
template <typename Ret, typename... Args>
struct fn_kind<Ret (*)(Args...) noexcept>
		: std::integral_constant<fn_kind_e, fn_kind_e::fn_ptr> {};
)

template <fn_kind_e Kind>
struct fn_view_impl;

template <>
struct fn_view_impl<fn_kind_e::fn_obj> {
	template <typename Fn>
	HEDLEY_ALWAYS_INLINE static auto address(Fn& arg) noexcept -> state_ptr {
		using char_ref = char&;
		using vptr = void*;
		using ptr = Fn*;
		return {ptr(vptr(&char_ref(arg)))};
	}
	HEDLEY_ALWAYS_INLINE static auto is_null(state_ptr /*arg*/) noexcept -> bool {
		return false;
	}

	template <typename Fn, typename Ret, typename... Args>
	HEDLEY_ALWAYS_INLINE static auto call(state_ptr state, Args... args) noexcept(
			VEG_CONCEPT(nothrow_invocable<Fn&&, Args&&...>)) -> Ret {
		return discard_void<Ret>::apply(
				static_cast<Fn&&>(*static_cast<meta::uncvref_t<Fn>*>(state.ptr)),
				VEG_FWD(args)...);
	}
};
template <>
struct fn_view_impl<fn_kind_e::fn_ptr> {
	template <typename Fn>
	HEDLEY_ALWAYS_INLINE static auto address(Fn& arg) noexcept -> state_ptr {
		state_ptr rv;
		rv.fn = reinterpret_cast<void (*)()>(+arg);
		return rv;
	}
	HEDLEY_ALWAYS_INLINE static auto is_null(state_ptr arg) noexcept -> bool {
		return arg.fn == nullptr;
	}

	template <typename Fn, typename Ret, typename... Args>
	HEDLEY_ALWAYS_INLINE static auto call(state_ptr state, Args... args) noexcept(
			VEG_CONCEPT(nothrow_invocable<Fn&&, Args&&...>)) -> Ret {
		return discard_void<Ret>::apply(
				reinterpret_cast<decltype(+VEG_DECLVAL(Fn&))>(state.fn),
				VEG_FWD(args)...);
	}
};

template <bool No_Except, typename Ret, typename... Args>
struct function_ref_impl {
	using raw_parts = fn::raw_parts<
			meta::conditional_t<No_Except, nothrow<Ret(Args...)>, Ret(Args...)>>;
	raw_parts self = {};

	constexpr function_ref_impl() = default;
	// COMPAT: check if function_ref_impl is a base of Fn
	template <typename Fn>
	HEDLEY_ALWAYS_INLINE
	function_ref_impl(char /*unused*/, char /*unused*/, Fn&& fn) noexcept
			: self{
						fnref::fn_view_impl<
								fnref::fn_kind<meta::decay_t<Fn>>::value>::address(fn),
						fnref::fn_view_impl<fnref::fn_kind<meta::decay_t<Fn>>::value>::
								template call<Fn, Ret, Args...>,
				} {
		bool null_fn_ptr =
				fnref::fn_view_impl<fnref::fn_kind<meta::decay_t<Fn>>::value>::is_null(
						self.state);
		VEG_ASSERT(!null_fn_ptr);
	}

	HEDLEY_ALWAYS_INLINE
	function_ref_impl(char /*unused*/, raw_parts parts) noexcept : self{parts} {}

	HEDLEY_ALWAYS_INLINE
	auto call_fn(Args... args) const noexcept(No_Except) -> Ret {
		void* fn_ptr = reinterpret_cast<void*>(self.fn_ptr);
		VEG_INTERNAL_ASSERT(fn_ptr != nullptr);
		return self.fn_ptr(self.state, VEG_FWD(args)...);
	}

	template <fnref::fn_kind_e>
	friend struct fnref::fn_view_impl;
};

} // namespace fnref
} // namespace internal

template <typename Ret, typename... Args>
struct fn_view<Ret(Args...)>
		: private internal::fnref::function_ref_impl<false, Ret, Args...> {
private:
	using base = internal::fnref::function_ref_impl<false, Ret, Args...>;
	explicit constexpr fn_view(unsafe_t /*unused*/) noexcept : base(){};
	template <typename T>
	friend struct meta::value_sentinel_for;
	template <typename T>
	friend struct fn_view;
	template <typename T>
	friend struct once_fn_view;

public:
	fn_view() = delete;

	VEG_TEMPLATE(
			(typename Fn),
			requires(
					!VEG_CONCEPT(base_of<meta::uncvref_t<Fn>, fn_view>) &&
					VEG_CONCEPT(invocable<Fn&, Args&&...>) &&
					(VEG_CONCEPT(void_type<Ret>) ||
	         VEG_CONCEPT(same<Ret, meta::invoke_result_t<Fn&, Args&&...>>))),
			HEDLEY_ALWAYS_INLINE fn_view, // NOLINT
			(fn, Fn&&))
	noexcept
			: base({}, {}, fn) {}

	using typename base::raw_parts;
	HEDLEY_ALWAYS_INLINE
	fn_view(from_raw_parts_t /*tag*/, raw_parts parts, unsafe_t /*tag*/) noexcept
			: base{{}, parts} {}
	auto into_raw_parts() const noexcept -> raw_parts { return base::self; }

	HEDLEY_ALWAYS_INLINE
	fn_view /* NOLINT */ (fn_view<nothrow<Ret(Args...)>> fn) noexcept
			: base({}, {fn.self.state, fn.self.fn_ptr}) {}

	HEDLEY_ALWAYS_INLINE
	auto operator()(Args... args) const -> Ret {
		return this->call_fn(VEG_FWD(args)...);
	}
};

template <typename Ret, typename... Args>
struct once_fn_view<Ret(Args...)>
		: private internal::fnref::function_ref_impl<false, Ret, Args...>,
			meta::internal::nocopy_ctor,
			meta::internal::nocopy_assign {
private:
	using base = internal::fnref::function_ref_impl<false, Ret, Args...>;
	explicit constexpr once_fn_view(unsafe_t /*unused*/) noexcept : base(){};
	template <typename T>
	friend struct meta::value_sentinel_for;
	template <typename T>
	friend struct fn_view;
	template <typename T>
	friend struct once_fn_view;

public:
	once_fn_view() = delete;

	VEG_TEMPLATE(
			(typename Fn),
			requires(
					!VEG_CONCEPT(base_of<meta::uncvref_t<Fn>, once_fn_view>) &&
					VEG_CONCEPT(invocable<Fn&&, Args&&...>) &&
					(VEG_CONCEPT(void_type<Ret>) ||
	         VEG_CONCEPT(same<Ret, meta::invoke_result_t<Fn&&, Args&&...>>))),
			HEDLEY_ALWAYS_INLINE once_fn_view, // NOLINT
			(fn, Fn&&))
	noexcept
			: base({}, {}, VEG_FWD(fn)) {}

	using typename base::raw_parts;
	HEDLEY_ALWAYS_INLINE
	once_fn_view(
			from_raw_parts_t /*tag*/, raw_parts parts, unsafe_t /*tag*/) noexcept
			: base{{}, parts} {}
	auto into_raw_parts() const&& noexcept -> raw_parts { return base::self; }

	HEDLEY_ALWAYS_INLINE
	once_fn_view /* NOLINT */ (fn_view<nothrow<Ret(Args...)>> fn) noexcept
			: base({}, {fn.self.state, fn.self.fn_ptr}) {}

	HEDLEY_ALWAYS_INLINE
	once_fn_view /* NOLINT */ (fn_view<Ret(Args...)> fn) noexcept
			: base({}, {fn.self.state, fn.self.fn_ptr}) {}

	HEDLEY_ALWAYS_INLINE
	auto operator()(Args... args) const&& -> Ret {
		return this->call_fn(VEG_FWD(args)...);
	}
};

template <typename Ret, typename... Args>
struct fn_view<nothrow<Ret(Args...)>>
		: private internal::fnref::function_ref_impl<true, Ret, Args...> {
private:
	using base = internal::fnref::function_ref_impl<true, Ret, Args...>;
	explicit constexpr fn_view(unsafe_t /*unused*/) noexcept : base(){};
	template <typename T>
	friend struct meta::value_sentinel_for;
	template <typename T>
	friend struct fn_view;
	template <typename T>
	friend struct once_fn_view;

public:
	fn_view() = delete;

	VEG_TEMPLATE(
			(typename Fn),
			requires(
					!VEG_CONCEPT(base_of<meta::uncvref_t<Fn>, fn_view>) &&
					VEG_CONCEPT(nothrow_invocable<Fn&, Args&&...>) &&
					(VEG_CONCEPT(void_type<Ret>) ||
	         VEG_CONCEPT(same<Ret, meta::invoke_result_t<Fn&, Args&&...>>))),
			HEDLEY_ALWAYS_INLINE fn_view, // NOLINT
			(fn, Fn&&))
	noexcept
			: base({}, {}, fn) {}

	using typename base::raw_parts;
	HEDLEY_ALWAYS_INLINE
	fn_view(from_raw_parts_t /*tag*/, raw_parts parts, unsafe_t /*tag*/) noexcept
			: base{{}, parts} {}
	auto into_raw_parts() const noexcept -> raw_parts { return base::self; }

	HEDLEY_ALWAYS_INLINE
	auto operator()(Args... args) const noexcept -> Ret {
		return this->call_fn(VEG_FWD(args)...);
	}
};

template <typename Ret, typename... Args>
struct once_fn_view<nothrow<Ret(Args...)>>
		: private internal::fnref::function_ref_impl<true, Ret, Args...>,
			meta::internal::nocopy_ctor,
			meta::internal::nocopy_assign {
private:
	using base = internal::fnref::function_ref_impl<true, Ret, Args...>;
	explicit constexpr once_fn_view(unsafe_t /*unused*/) noexcept : base(){};
	template <typename T>
	friend struct meta::value_sentinel_for;
	template <typename T>
	friend struct fn_view;
	template <typename T>
	friend struct once_fn_view;

public:
	once_fn_view() = delete;

	VEG_TEMPLATE(
			(typename Fn),
			requires(
					!VEG_CONCEPT(base_of<meta::uncvref_t<Fn>, once_fn_view>) &&
					VEG_CONCEPT(nothrow_invocable<Fn&&, Args&&...>) &&
					(VEG_CONCEPT(void_type<Ret>) ||
	         VEG_CONCEPT(same<Ret, meta::invoke_result_t<Fn&&, Args&&...>>))),
			HEDLEY_ALWAYS_INLINE once_fn_view, // NOLINT
			(fn, Fn&&))
	noexcept
			: base({}, {}, VEG_FWD(fn)) {}

	using typename base::raw_parts;
	HEDLEY_ALWAYS_INLINE
	once_fn_view(
			from_raw_parts_t /*tag*/, raw_parts parts, unsafe_t /*tag*/) noexcept
			: base{{}, parts} {}
	auto into_raw_parts() const&& noexcept -> raw_parts { return base::self; }

	HEDLEY_ALWAYS_INLINE
	once_fn_view /* NOLINT */ (fn_view<Ret(Args...) noexcept> fn) noexcept
			: base({}, {fn.self.state, fn.self.fn_ptr}) {}

	HEDLEY_ALWAYS_INLINE
	auto operator()(Args... args) const&& noexcept -> Ret {
		return this->call_fn(VEG_FWD(args)...);
	}
};

} // namespace fn

template <typename T>
struct meta::value_sentinel_for<fn::fn_view<T>>
		: std::integral_constant<i64, 1> {
	HEDLEY_ALWAYS_INLINE
	static constexpr auto invalid(i64 i) noexcept -> fn::fn_view<T> {
		return (void)i, VEG_INTERNAL_ASSERT(i == i64(0)), fn::fn_view<T>{unsafe};
	}
	HEDLEY_ALWAYS_INLINE
	static constexpr auto id(fn::fn_view<T> arg) -> i64 {
		return arg.self.fn_ptr == nullptr ? 0 : -1;
	}
};
template <typename T>
struct meta::value_sentinel_for<fn::once_fn_view<T>>
		: std::integral_constant<i64, 1> {
	HEDLEY_ALWAYS_INLINE
	static constexpr auto invalid(i64 i) noexcept -> fn::once_fn_view<T> {
		return (void)i, VEG_INTERNAL_ASSERT(i == i64(0)),
					 fn::once_fn_view<T>{unsafe};
	}
	HEDLEY_ALWAYS_INLINE
	static constexpr auto id(fn::once_fn_view<T> arg) -> i64 {
		return arg.self.fn_ptr == nullptr ? 0 : -1;
	}
};
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard __VEG_FN_REF_HPP_IDVWA3EYS */
