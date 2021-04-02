#ifndef VEG_FN_VIEW_HPP_2EMSNQIXS
#define VEG_FN_VIEW_HPP_2EMSNQIXS

#include "veg/util/assert.hpp"
#include "veg/type_traits/cpo/tombstone.hpp"
#include "veg/internal/delete_special_members.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {

inline namespace VEG_ABI {
namespace internal {
namespace fnref {

// veg::@class
union state_ptr {
	void* ptr;
	void (*fn)();
};

template <bool No_Except, typename Ret, typename... Args>
struct raw_parts {
	state_ptr state;
	auto (*fn_ptr)(state_ptr, Args...) -> Ret;
};

template <typename Ret, typename... Args>
struct raw_parts<true, Ret, Args...> {
	state_ptr state;
	auto (*fn_ptr)(state_ptr, Args...) noexcept -> Ret;
};

template <typename T>
struct is_fn_ptr : meta::false_type {};
template <typename Ret, typename... Args>
struct is_fn_ptr<Ret (*)(Args...)> : meta::true_type {};
VEG_CPP17(
template <typename Ret, typename... Args>
struct is_fn_ptr<Ret (*)(Args...)noexcept> : meta::true_type {};
)
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
			VEG_CONCEPT(nothrow_invocable<Fn, Args&&...>)) -> Ret {
		return VEG_FWD(fn)(VEG_FWD(args)...);
	}
};
template <>
struct discard_void<void> {
	template <typename Fn, typename... Args>
	HEDLEY_ALWAYS_INLINE static auto apply(Fn&& fn, Args&&... args) noexcept(
			VEG_CONCEPT(nothrow_invocable<Fn, Args&&...>)) -> void {
		void(VEG_FWD(fn)(VEG_FWD(args)...));
	}
};

enum struct fn_kind_e {
	fn_ptr,
	fn_obj,
};

template <typename T>
struct fn_kind : meta::constant<
										 fn_kind_e,
										 is_convertible_to_fn_ptr_with_unary_plus<T>::value
												 ? fn_kind_e::fn_ptr
												 : fn_kind_e::fn_obj> {};

template <typename Ret, typename... Args>
struct fn_kind<Ret (*)(Args...)>
		: meta::constant<fn_kind_e, fn_kind_e::fn_ptr> {};

VEG_CPP17(
template <typename Ret, typename... Args>
struct fn_kind<Ret (*)(Args...) noexcept>
    : meta::constant<fn_kind_e, fn_kind_e::fn_ptr> {};
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

	template <typename Fn, typename Ret, typename... Args>
	HEDLEY_ALWAYS_INLINE static auto call(state_ptr state, Args... args) noexcept(
			VEG_CONCEPT(nothrow_invocable<Fn, Args&&...>)) -> Ret {
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

	template <typename Fn, typename Ret, typename... Args>
	HEDLEY_ALWAYS_INLINE static auto call(state_ptr state, Args... args) noexcept(
			VEG_CONCEPT(nothrow_invocable<Fn, Args&&...>)) -> Ret {
		return discard_void<Ret>::apply(
				reinterpret_cast<decltype(+VEG_DECLVAL(Fn&))>(state.fn),
				VEG_FWD(args)...);
	}
};

struct assert_non_null_ref {
	HEDLEY_ALWAYS_INLINE
	static void apply(state_ptr /*arg*/) noexcept {}
};
struct assert_non_null_ptr {
	HEDLEY_ALWAYS_INLINE
	static void apply(state_ptr arg) noexcept {
		VEG_ASSERT(bool(arg.fn != nullptr));
	}
};
template <typename T>
struct assert_non_null : assert_non_null_ref {};
template <typename Ret, typename... Args>
struct assert_non_null<Ret (*)(Args...)> : assert_non_null_ptr {};

template <bool No_Except, typename Ret, typename... Args>
struct function_ref_impl {
	using raw_parts = fnref::raw_parts<No_Except, Ret, Args...>;
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
		assert_non_null<meta::uncvref_t<Fn>>::apply(self.state);
	}

	HEDLEY_ALWAYS_INLINE
	function_ref_impl(char /*unused*/, raw_parts parts) noexcept : self{parts} {}

	HEDLEY_ALWAYS_INLINE
	auto call_fn(Args... args) const noexcept(No_Except) -> Ret {
		auto* fn_ptr = reinterpret_cast<void (*)()>(self.fn_ptr);
		(void)fn_ptr;
		VEG_INTERNAL_ASSERT(bool(fn_ptr != nullptr));
		return self.fn_ptr(self.state, VEG_FWD(args)...);
	}

	template <fnref::fn_kind_e>
	friend struct fnref::fn_view_impl;
};

} // namespace fnref
} // namespace internal

namespace fn {

template <typename T>
struct fn_view;
template <typename T>
struct fn_once_view;

template <typename Ret, typename... Args>
struct fn_view<Ret(Args...)>
		: private internal::fnref::function_ref_impl<false, Ret, Args...> {
private:
	using base = internal::fnref::function_ref_impl<false, Ret, Args...>;
	explicit constexpr fn_view(unsafe_t /*unused*/) noexcept : base(){};
	template <typename T>
	friend struct meta::tombstone_traits;
	template <typename T>
	friend struct fn_view;
	template <typename T>
	friend struct fn_once_view;

public:
	fn_view() = delete;

	VEG_TEMPLATE(
			(typename Fn),
			requires(
					!VEG_CONCEPT(constructible<fn_view*, meta::uncvref_t<Fn>*>) &&
					VEG_CONCEPT(invocable_r<Fn&, Ret, Args&&...>)),
			HEDLEY_ALWAYS_INLINE fn_view,
			(fn, Fn&&))
	noexcept
			: base({}, {}, fn) {}

	using typename base::raw_parts;
	HEDLEY_ALWAYS_INLINE
	fn_view(from_raw_parts_t /*tag*/, raw_parts parts, unsafe_t /*tag*/) noexcept
			: base{{}, parts} {}
	auto into_raw_parts() const noexcept -> raw_parts { return base::self; }

#if __cplusplus >= 201703L
	HEDLEY_ALWAYS_INLINE
	fn_view(fn_view<Ret(Args...) noexcept> fn) noexcept
			: base({}, {fn.self.state, fn.self.fn_ptr}) {}
#endif

	HEDLEY_ALWAYS_INLINE
	auto operator()(Args... args) const -> Ret {
		return this->call_fn(VEG_FWD(args)...);
	}
};

template <typename Ret, typename... Args>
struct fn_once_view<Ret(Args...)>
		: private internal::fnref::function_ref_impl<false, Ret, Args...>,
			internal::nocopy_ctor,
			internal::nocopy_assign {
private:
	using base = internal::fnref::function_ref_impl<false, Ret, Args...>;
	explicit constexpr fn_once_view(unsafe_t /*unused*/) noexcept : base(){};
	template <typename T>
	friend struct meta::tombstone_traits;
	template <typename T>
	friend struct fn_view;
	template <typename T>
	friend struct fn_once_view;

public:
	fn_once_view() = delete;

	VEG_TEMPLATE(
			(typename Fn),
			requires(
					!VEG_CONCEPT(constructible<
											 fn_once_view*,
											 meta::uncvref_t<Fn>*,
											 fn_once_view>) &&
					VEG_CONCEPT(invocable_r<Fn, Ret, Args&&...>)),
			HEDLEY_ALWAYS_INLINE fn_once_view,
			(fn, Fn&&))
	noexcept
			: base({}, {}, VEG_FWD(fn)) {}

	using typename base::raw_parts;
	HEDLEY_ALWAYS_INLINE
	fn_once_view(
			from_raw_parts_t /*tag*/, raw_parts parts, unsafe_t /*tag*/) noexcept
			: base{{}, parts} {}
	auto into_raw_parts() const&& noexcept -> raw_parts { return base::self; }

#if __cplusplus >= 201703L
	HEDLEY_ALWAYS_INLINE
	fn_once_view(fn_view<Ret(Args...) noexcept> fn) noexcept
			: base({}, {fn.self.state, fn.self.fn_ptr}) {}
#endif

	HEDLEY_ALWAYS_INLINE
	fn_once_view(fn_view<Ret(Args...)> fn) noexcept
			: base({}, {fn.self.state, fn.self.fn_ptr}) {}

	HEDLEY_ALWAYS_INLINE
	auto operator()(Args... args) const&& -> Ret {
		return this->call_fn(VEG_FWD(args)...);
	}
};

#if __cplusplus >= 201703L
template <typename Ret, typename... Args>
struct fn_view<Ret(Args...) noexcept>
		: private internal::fnref::function_ref_impl<true, Ret, Args...> {
private:
	using base = internal::fnref::function_ref_impl<true, Ret, Args...>;
	explicit constexpr fn_view(unsafe_t /*unused*/) noexcept : base(){};
	template <typename T>
	friend struct meta::tombstone_traits;
	template <typename T>
	friend struct fn_view;
	template <typename T>
	friend struct fn_once_view;

public:
	fn_view() = delete;

	VEG_TEMPLATE(
			(typename Fn),
			requires(
					!VEG_CONCEPT(constructible<fn_view*, meta::uncvref_t<Fn>*>) &&
					VEG_CONCEPT(nothrow_invocable_r<Fn&, Ret, Args&&...>)),
			HEDLEY_ALWAYS_INLINE fn_view,
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
struct fn_once_view<Ret(Args...) noexcept>
		: private internal::fnref::function_ref_impl<true, Ret, Args...>,
			internal::nocopy_ctor,
			internal::nocopy_assign {
private:
	using base = internal::fnref::function_ref_impl<true, Ret, Args...>;
	explicit constexpr fn_once_view(unsafe_t /*unused*/) noexcept : base(){};
	template <typename T>
	friend struct meta::tombstone_traits;
	template <typename T>
	friend struct fn_view;
	template <typename T>
	friend struct fn_once_view;

public:
	fn_once_view() = delete;

	VEG_TEMPLATE(
			(typename Fn),
			requires(
					!VEG_CONCEPT(constructible<
											 fn_once_view*,
											 meta::uncvref_t<Fn>*,
											 fn_once_view>) &&
					VEG_CONCEPT(nothrow_invocable_r<Fn, Ret, Args&&...>)),
			HEDLEY_ALWAYS_INLINE fn_once_view,
			(fn, Fn&&))
	noexcept
			: base({}, {}, VEG_FWD(fn)) {}

	using typename base::raw_parts;
	HEDLEY_ALWAYS_INLINE
	fn_once_view(
			from_raw_parts_t /*tag*/, raw_parts parts, unsafe_t /*tag*/) noexcept
			: base{{}, parts} {}
	auto into_raw_parts() const&& noexcept -> raw_parts { return base::self; }

	HEDLEY_ALWAYS_INLINE
	fn_once_view(fn_view<Ret(Args...) noexcept> fn) noexcept
			: base({}, {fn.self.state, fn.self.fn_ptr}) {}

	HEDLEY_ALWAYS_INLINE
	auto operator()(Args... args) const&& noexcept -> Ret {
		return this->call_fn(VEG_FWD(args)...);
	}
};
#endif

} // namespace fn

template <typename T>
struct meta::tombstone_traits<fn::fn_view<T>> {
	static constexpr i64 spare_representations = 1;

	HEDLEY_ALWAYS_INLINE
	static VEG_CPP14(constexpr) void set_spare_representation(
			fn::fn_view<T>* p, i64 i) noexcept {
		(void)i, VEG_INTERNAL_ASSERT(i == i64(0)), (*p = fn::fn_view<T>{unsafe});
	}
	HEDLEY_ALWAYS_INLINE
	static constexpr auto index(fn::fn_view<T> const* p) -> i64 {
		return p->self.fn_ptr == nullptr ? 0 : -1;
	}
};
template <typename T>
struct meta::tombstone_traits<fn::fn_once_view<T>> {
	static constexpr i64 spare_representations = 1;

	HEDLEY_ALWAYS_INLINE
	static VEG_CPP14(constexpr) void set_spare_representation(
			fn::fn_view<T>* p, i64 i) noexcept {
		(void)i, VEG_INTERNAL_ASSERT(i == i64(0)), (*p = fn::fn_view<T>{unsafe});
	}
	HEDLEY_ALWAYS_INLINE
	static constexpr auto index(fn::fn_view<T> const* p) -> i64 {
		return p->self.fn_ptr == nullptr ? 0 : -1;
	}
};
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"

#endif /* end of include guard VEG_FN_VIEW_HPP_2EMSNQIXS */
