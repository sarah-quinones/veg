#ifndef VEG_OPTION_HPP_8NVLXES2S
#define VEG_OPTION_HPP_8NVLXES2S

#include "veg/util/assert.hpp"
#include "veg/util/swap.hpp"
#include "veg/util/unreachable.hpp"
#include "veg/memory/placement.hpp"
#include "veg/memory/address.hpp"
#include "veg/type_traits/constructible.hpp"
#include "veg/util/compare.hpp"
#include "veg/internal/delete_special_members.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {

template <typename T>
struct Option;

namespace option {
namespace nb {
struct make {
	VEG_TEMPLATE(
			typename T,
			requires(VEG_CONCEPT(move_constructible<T>)),
			VEG_NODISCARD VEG_INLINE constexpr auto
			operator(),
			(arg, T))
	const VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_move_constructible<T>))->Option<T> {
		return {inplace, internal::ConvertingFn<T&&, T>{VEG_FWD(arg)}};
	}
};
struct some : make {};
struct fwd {
	VEG_TEMPLATE(
			typename T,
			requires(VEG_CONCEPT(move_constructible<T>)),
			VEG_NODISCARD VEG_INLINE constexpr auto
			operator(),
			(arg, T&&))
	const VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_move_constructible<T>))->Option<T> {
		return {inplace, internal::ConvertingFn<T&&, T>{VEG_FWD(arg)}};
	}
};

struct ref {
	template <typename T>
	VEG_NODISCARD VEG_INLINE constexpr auto operator()(T&& arg) const VEG_NOEXCEPT
			-> Option<T&&> {
		return {some{}, VEG_FWD(arg)};
	}
};
} // namespace nb
VEG_NIEBLOID(make);
VEG_NIEBLOID(some);
VEG_NIEBLOID(fwd);
VEG_NIEBLOID(ref);
} // namespace option

inline namespace tags {
using Some = veg::option::nb::some;
using veg::option::some;

struct None {
	friend constexpr auto operator==(None /*lhs*/, None /*rhs*/) VEG_NOEXCEPT
			-> bool {
		return true;
	}
	friend constexpr auto operator!=(None /*lhs*/, None /*rhs*/) VEG_NOEXCEPT
			-> bool {
		return false;
	}
};
VEG_INLINE_VAR(none, None);
} // namespace tags

namespace cpo {
template <typename T>
struct is_trivially_constructible<Option<T>> : is_trivially_constructible<T> {};
template <typename T>
struct is_trivially_relocatable<Option<T>> : is_trivially_relocatable<T> {};
template <typename T>
struct is_trivially_swappable<Option<T>> : is_trivially_swappable<T> {};
} // namespace cpo

namespace meta {
template <typename T>
struct is_option : false_type {};
template <typename T>
struct is_option<Option<T>> : true_type {};

template <typename T>
struct option_type : type_identity<internal::meta_::none> {};
template <typename T>
struct option_type<Option<T>> : type_identity<T> {};

template <typename T>
using option_type_t = typename option_type<T>::type;
} // namespace meta

namespace concepts {
VEG_DEF_CONCEPT(typename T, option, meta::is_option<T>::value);
} // namespace concepts

namespace internal {
template <typename Fn, typename T>
struct WithArg {
	Fn&& fn;
	T&& arg;
	VEG_INLINE constexpr auto
	operator()() const&& -> meta::invoke_result_t<Fn, T> {
		return VEG_FWD(fn)(VEG_FWD(arg));
	}
};

namespace option_ {

struct hidden_tag {};

enum triviality_e { reference, all_empty_nonfinal, all, copyable, no };

template <typename T, triviality_e Kind>
struct OptionStorage;

template <typename T>
struct OptionStorage<T, triviality_e::reference>
		: NoCopyAssign,
			NoMoveAssign,
			meta::conditional_t<VEG_CONCEPT(rvalue_reference<T>), NoCopyCtor, Empty> {
	using Pointer = meta::unref_t<T>*;

	Pointer inner = nullptr;

	OptionStorage() = default;
	VEG_INLINE constexpr OptionStorage(T val) VEG_NOEXCEPT
			: inner(mem::addressof(val)) {}

	template <typename Fn>
	VEG_INLINE constexpr OptionStorage(Fn fn)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_invocable<Fn>))
			: OptionStorage(VEG_FWD(fn)()) {}

	VEG_NODISCARD VEG_INLINE constexpr auto is_engaged() const VEG_NOEXCEPT
			-> bool {
		return inner != nullptr;
	}

	VEG_INLINE
	VEG_CPP14(constexpr) void _destroy() VEG_NOEXCEPT {
		meta::unreachable_if(!is_engaged());
		inner = nullptr;
	}

	template <typename Fn>
	VEG_INLINE VEG_CPP14(constexpr) void _emplace(Fn fn)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_invocable<Fn>)) {
		meta::unreachable_if(is_engaged());
		T a = VEG_FWD(fn)();
		inner = mem::addressof(a);
	}

	constexpr auto _get() const noexcept -> T& { return *inner; }
};

template <bool Assignable>
struct emplace_fn;

template <>
struct emplace_fn<true> {
	template <typename T>
	VEG_INLINE VEG_CPP14(constexpr) void
	operator()(T& lhs, T&& rhs) VEG_NOEXCEPT {
		lhs = VEG_FWD(rhs);
	}
};
template <>
struct emplace_fn<false> {
	template <typename T>
	VEG_INLINE VEG_CPP20(constexpr) void
	operator()(T& lhs, T&& rhs) VEG_NOEXCEPT {
		mem::construct_at(mem::addressof(lhs), VEG_FWD(rhs));
	}
};

template <typename T>
struct OptionStorage<T, triviality_e::all_empty_nonfinal> {
	struct Inner : T {
		bool engaged = false;

		Inner() = default;

		template <typename Fn>
		VEG_INLINE constexpr Inner(Fn fn)
				VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_invocable<Fn>))
				: T(VEG_FWD(fn)()), engaged{true} {}
	} inner;

	OptionStorage() = default;

	template <typename Fn>
	VEG_INLINE constexpr OptionStorage(Fn fn)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_invocable<Fn>))
			: inner(VEG_FWD(fn)) {}

	VEG_NODISCARD VEG_INLINE constexpr auto is_engaged() const VEG_NOEXCEPT
			-> bool {
		return inner.engaged;
	}

	VEG_INLINE
	VEG_CPP14(constexpr) void _destroy() VEG_NOEXCEPT {
		meta::unreachable_if(!is_engaged());
		inner.engaged = false;
	}

	template <typename Fn>
	VEG_INLINE VEG_CPP14(constexpr) void _emplace(Fn fn)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_invocable<Fn>)) {
		meta::unreachable_if(is_engaged());
		emplace_fn<VEG_CONCEPT(assignable<T&, T&&>)>{}(
				static_cast<T&>(inner), VEG_FWD(fn)());
		inner.engaged = true;
	}

	constexpr auto _get() const noexcept -> T& { return const_cast<T&>(inner); }
};

template <typename T>
struct OptionStorage<T, triviality_e::all> {
	T inner{};
	bool engaged = false;

	OptionStorage() = default;

	template <typename Fn>
	VEG_INLINE constexpr OptionStorage(Fn fn)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_invocable<Fn>))
			: inner(VEG_FWD(fn)()), engaged{true} {}

	VEG_NODISCARD VEG_INLINE constexpr auto is_engaged() const VEG_NOEXCEPT
			-> bool {
		return engaged;
	}

	VEG_INLINE
	VEG_CPP14(constexpr) void _destroy() VEG_NOEXCEPT {
		meta::unreachable_if(!is_engaged());
		engaged = false;
	}

	template <typename Fn>
	VEG_INLINE VEG_CPP14(constexpr) void _emplace(Fn fn)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_invocable<Fn>)) {
		meta::unreachable_if(is_engaged());
		emplace_fn<VEG_CONCEPT(assignable<T&, T&&>)>{}(inner, VEG_FWD(fn)());
		engaged = true;
	}

	constexpr auto _get() const noexcept -> T& { return const_cast<T&>(inner); }
};

template <typename T>
struct OptionStorage<T, triviality_e::copyable> {
	union {
		Empty e{};
		T inner;
	};
	bool engaged = false;

	constexpr OptionStorage /* NOLINT */ () VEG_NOEXCEPT{};

	template <typename Fn>
	VEG_INLINE constexpr OptionStorage(Fn fn)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_invocable<Fn>))
			: inner(VEG_FWD(fn)()), engaged{true} {}

	VEG_NODISCARD VEG_INLINE constexpr auto is_engaged() const VEG_NOEXCEPT
			-> bool {
		return engaged;
	}

	VEG_INLINE
	VEG_CPP14(constexpr) void _destroy() VEG_NOEXCEPT {
		meta::unreachable_if(!is_engaged());
		engaged = false;
	}

	template <typename Fn>
	VEG_INLINE VEG_CPP14(constexpr) void _emplace(Fn fn)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_invocable<Fn>)) {
		meta::unreachable_if(is_engaged());
		emplace_fn<VEG_CONCEPT(assignable<T&, T&&>)>{}(inner, VEG_FWD(fn)());
		engaged = true;
	}

	constexpr auto _get() const noexcept -> T& { return const_cast<T&>(inner); }
};

#define VEG_CONSTEXPR VEG_CPP20(constexpr)
template <typename T>
struct OptionStorageBase {
	union {
		Empty e{};
		T inner;
	};
	bool engaged = false;

	VEG_CONSTEXPR OptionStorageBase /* NOLINT */ () VEG_NOEXCEPT{};

	template <typename Fn>
	VEG_INLINE VEG_CONSTEXPR OptionStorageBase(Fn fn)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_invocable<Fn>))
			: inner(VEG_FWD(fn)()), engaged{true} {}

	VEG_CONSTEXPR
	~OptionStorageBase() VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_destructible<T>)) {
		if (engaged) {
			_destroy();
		}
	}

	VEG_CONSTEXPR
	OptionStorageBase(OptionStorageBase&& rhs)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_move_constructible<T>)) {
		if (rhs.engaged) {
			_emplace(ConvertingFn<T&&, T>{VEG_FWD(rhs.inner)});
		}
	}
	VEG_CONSTEXPR
	OptionStorageBase(OptionStorageBase const& rhs)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_copy_constructible<T>)) {
		if (rhs.engaged) {
			_emplace(ConvertingFn<T const&, T>{rhs.inner});
		}
	}

	VEG_CONSTEXPR
	auto operator=(OptionStorageBase&& rhs) VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_move_constructible<T>) &&
			VEG_CONCEPT(nothrow_move_assignable<T>)) -> OptionStorageBase& {
		if (engaged) {
			if (rhs.engaged) {
				inner = VEG_FWD(rhs.inner);
			} else {
				_destroy();
			}
		} else {
			if (rhs.engaged) {
				_emplace(ConvertingFn<T&&, T>{VEG_FWD(rhs.inner)});
			}
		}
		return *this;
	}
	VEG_CONSTEXPR
	auto operator=(OptionStorageBase const& rhs) VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_copy_constructible<T>) &&
			VEG_CONCEPT(nothrow_copy_assignable<T>)) -> OptionStorageBase& {
		if (engaged) {
			if (rhs.engaged) {
				inner = rhs.inner;
			} else {
				_destroy();
			}
		} else {
			if (rhs.engaged) {
				_emplace(ConvertingFn<T const&, T>{rhs.inner});
			}
		}
		return *this;
	}

	VEG_NODISCARD VEG_INLINE VEG_CONSTEXPR auto is_engaged() const VEG_NOEXCEPT
			-> bool {
		return engaged;
	}

	VEG_INLINE
	VEG_CONSTEXPR
	void _destroy() VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_destructible<T>)) {
		meta::unreachable_if(!is_engaged());
		engaged = false;
		mem::destroy_at(mem::addressof(inner));
	}

	template <typename Fn>
	VEG_INLINE VEG_CONSTEXPR void _emplace(Fn fn)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_invocable<Fn>)) {
		meta::unreachable_if(is_engaged());
		mem::construct_with(mem::addressof(inner), VEG_FWD(fn));
		engaged = true;
	}

	VEG_CONSTEXPR auto _get() const noexcept -> T& {
		return const_cast<T&>(inner);
	}
};
#undef VEG_CONSTEXPR

template <typename T>
struct OptionStorage<T, triviality_e::no>
		: OptionStorageBase<T>,

			meta::conditional_t<
					VEG_CONCEPT(move_constructible<T>),
					EmptyI<0>,
					NoMoveCtor>,
			meta::conditional_t<
					VEG_CONCEPT(copy_constructible<T>),
					EmptyI<1>,
					NoCopyCtor>,
			meta::conditional_t<
					VEG_CONCEPT(move_constructible<T>) && VEG_CONCEPT(move_assignable<T>),
					EmptyI<2>,
					NoMoveAssign>,
			meta::conditional_t<
					VEG_CONCEPT(copy_constructible<T>) && VEG_CONCEPT(copy_assignable<T>),
					EmptyI<3>,
					NoCopyAssign>

{
	using OptionStorageBase<T>::OptionStorageBase;
};

template <typename To>
struct into_fn {
	template <typename T>
	VEG_INLINE constexpr auto operator()(T&& ref) const
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_constructible<To, T&&>)) -> To {
		return VEG_FWD(ref);
	}
};

VEG_TEMPLATE(
		(typename T, typename U),
		requires(VEG_CONCEPT(equality_comparable_with<T const&, U const&>)),
		VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto
		operator==,
		(lhs, Option<T> const&),
		(rhs, Option<U> const&))
VEG_NOEXCEPT->bool {
	if (lhs) {
		if (rhs) {
			return static_cast<bool>(
					lhs.as_cref().unwrap_unchecked(unsafe) ==
					rhs.as_cref().unwrap_unchecked(unsafe));
		}
	}
	return (static_cast<bool>(lhs) == static_cast<bool>(rhs));
}

VEG_TEMPLATE(
		(typename T, typename U),
		requires(VEG_CONCEPT(equality_comparable_with<T const&, U const&>)),
		VEG_INLINE VEG_CPP14(constexpr) auto
		operator!=,
		(a, Option<T> const&),
		(b, Option<U> const&))
VEG_NOEXCEPT->bool {
	return !(a == b);
}

template <typename T>
struct ret_none {
	VEG_INLINE VEG_CPP14(constexpr) auto operator()() const VEG_NOEXCEPT -> T {
		return none;
	}
};

template <typename U>
struct cmp_with_fn {
	U const& rhs;
	template <typename T>
	VEG_INLINE constexpr auto operator()(T const& lhs) const VEG_NOEXCEPT
			-> bool {
		return veg::cmp::equal(lhs, rhs);
	}
};

template <typename T>
struct choose_base_impl {
	using Type = OptionStorage<
			T,
			!VEG_CONCEPT(trivially_copyable<T>)                           //
					? triviality_e::no                                        //
					: !cpo::is_trivially_constructible<T>::value              //
								? triviality_e::copyable                            //
								: !(VEG_CONCEPT(empty<T>) && VEG_CONCEPT(final<T>)) //
											? triviality_e::all                           //
											: triviality_e::all_empty_nonfinal            //
			>;
};

template <typename T>
struct choose_base_impl<T&> {
	using Type = OptionStorage<T&, triviality_e::reference>;
};
template <typename T>
struct choose_base_impl<T&&> {
	using Type = OptionStorage<T&&, triviality_e::reference>;
};

template <typename T>
using choose_base = typename choose_base_impl<T>::Type;

} // namespace option_
} // namespace internal

template <typename T>
struct VEG_NODISCARD Option : private internal::option_::choose_base<T> {
private:
	using Base = internal::option_::choose_base<T>;

public:
	Option() = default;
	VEG_DEFAULT_CTOR_ASSIGN(Option);

	constexpr Option // NOLINT(hicpp-explicit-conversions)
			(None /*tag*/) VEG_NOEXCEPT : Option{} {}

	VEG_CONSTRAINED_MEMBER_FN(
			requires(VEG_CONCEPT(move_constructible<T>)),
			VEG_INLINE constexpr Option,
			((/*tag*/, Some), (value, T)),
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_move_constructible<T>)))
			: Base(internal::ConvertingFn<T&&, T>{VEG_FWD(value)}) {}

	VEG_TEMPLATE(
			(typename Fn),
			requires(VEG_CONCEPT(invocable_r<Fn, T>)),
			VEG_INLINE constexpr Option,
			(/*tag*/, InPlace),
			(fn, Fn))
	VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_invocable<Fn>))
			: internal::option_::choose_base<T>(VEG_FWD(fn)) {}

	VEG_CPP14(constexpr)
	auto operator=(None /*arg*/)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_destructible<T>)) -> Option& {
		if (*this) {
			this->_destroy();
		}
		return *this;
	}

	VEG_CONSTRAINED_MEMBER_FN(
			requires(VEG_CONCEPT(option<T>)),
			VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto flatten,
			((/*tag*/ = {}, Safe)),
			&&VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_move_constructible<T>))->T) {
		return static_cast<Option<T>&&>(*this).map_or_else(
				internal::option_::into_fn<T>{}, internal::option_::ret_none<T>{});
	}

	VEG_CONSTRAINED_MEMBER_FN(
			requires(VEG_CONCEPT(move_constructible<T>)),
			VEG_NODISCARD VEG_CPP14(constexpr) auto take,
			((/*tag*/ = {}, Safe)),
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_move_constructible<T>))->Option<T>) {
		if (this->is_engaged()) {
			Option<T> val{
					inplace,
					internal::ConvertingFn<T&&, T>{static_cast<T&&>(this->_get())},
			};
			*this = none;
			return val;
		}
		return none;
	}

	VEG_CONSTRAINED_MEMBER_FN(
			requires(VEG_CONCEPT(move_constructible<T>)),
			VEG_NODISCARD VEG_CPP14(constexpr) auto unwrap_unchecked,
			((/*tag*/, Unsafe)),
			&&VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_move_constructible<T>))->T) {
		meta::unreachable_if(!bool(*this));
		return static_cast<T&&>(this->_get());
	}

	VEG_CONSTRAINED_MEMBER_FN(
			requires(VEG_CONCEPT(move_constructible<T>)),
			VEG_NODISCARD VEG_CPP14(constexpr) auto unwrap,
			((/*tag*/ = {}, Safe)),
			&&VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_move_constructible<T>))->T) {
		VEG_INTERNAL_ASSERT_PRECONDITION(*this);
		return static_cast<T&&>(this->_get());
	}

	VEG_TEMPLATE(
			(typename Fn),
			requires(
					VEG_CONCEPT(move_constructible<T>) &&
					VEG_CONCEPT(invocable_r<Fn, bool, meta::uncvref_t<T> const&>)),
			VEG_NODISCARD VEG_CPP14(constexpr) auto filter,
			(fn, Fn&&)) &&

			VEG_NOEXCEPT_IF(
					(VEG_CONCEPT(nothrow_invocable<Fn&&, meta::uncvref_t<T> const&>) &&
	         VEG_CONCEPT(nothrow_move_constructible<T>))) -> Option<T> {
		auto&& self = static_cast<Option<T>&&>(*this);
		if (self) {
			if (VEG_FWD(fn)(self.as_cref().unwrap_unchecked(unsafe))) {
				return {
						inplace,
						internal::ConvertingFn<T&&, T>{static_cast<T&&>(this->_get())},
				};
			}
		}
		return none;
	}

	VEG_TEMPLATE(
			typename U,
			requires(VEG_CONCEPT(equality_comparable_with<T const&, U const&>)),
			VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto contains,
			(val, U const&))
	const VEG_NOEXCEPT->bool {
		return as_ref().map_or(internal::option_::cmp_with_fn<U>{val}, false);
	}

	VEG_TEMPLATE(
			typename Fn,
			requires(VEG_CONCEPT(invocable_r<Fn, T>)),
			VEG_CPP14(constexpr) auto emplace,
			(fn, Fn)) &
			VEG_NOEXCEPT_IF(
					VEG_CONCEPT(nothrow_destructible<T>) &&
					VEG_CONCEPT(nothrow_invocable<Fn>)) -> T& {
		if (*this) {
			this->_destroy();
		}
		this->_emplace(VEG_FWD(fn));
		return this->_get();
	}

	VEG_TEMPLATE(
			(typename Fn),
			requires(VEG_CONCEPT(invocable<Fn, T&&>)),
			VEG_NODISCARD VEG_CPP14(constexpr) auto map,
			(fn, Fn&&)) &&

			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_invocable<Fn, T&&>))
					-> Option<meta::invoke_result_t<Fn, T>> {
		if (*this) {
			return Option<meta::invoke_result_t<Fn, T>>{
					inplace,
					internal::WithArg<Fn&&, T&&>{
							VEG_FWD(fn), static_cast<T&&>(this->_get())},
			};
		}
		return none;
	}

	VEG_TEMPLATE(
			(typename Fn),
			requires(
					VEG_CONCEPT(invocable<Fn, T&&>) &&
					VEG_CONCEPT(option<meta::invoke_result_t<Fn, T>>)),
			VEG_NODISCARD VEG_CPP14(constexpr) auto and_then,
			(fn, Fn&&)) &&

			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_invocable<Fn, T&&>))
					-> meta::invoke_result_t<Fn, T> {
		if (*this) {
			return VEG_FWD(fn)(static_cast<T&&>(this->_get()));
		}
		return none;
	}

	VEG_TEMPLATE(
			(typename Fn, typename D),
			requires(
					(VEG_CONCEPT(invocable<Fn, T&&>) && //
	         VEG_CONCEPT(invocable<D>) &&       //
	         VEG_CONCEPT(
							 same<meta::invoke_result_t<Fn, T>, meta::invoke_result_t<D>>))),
			VEG_NODISCARD VEG_CPP14(constexpr) auto map_or_else,
			(fn, Fn&&),
			(d, D&&)) &&

			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_invocable<Fn>))
					-> meta::invoke_result_t<Fn, T> {
		if (*this) {
			return VEG_FWD(fn)(
					static_cast<Option&&>(*this).as_ref().unwrap_unchecked(unsafe));
		}
		return VEG_FWD(d)();
	}

	VEG_TEMPLATE(
			(typename Fn),
			requires(
					VEG_CONCEPT(invocable<Fn, T&&>) &&
					VEG_CONCEPT(move_constructible<meta::invoke_result_t<Fn, T>>)),
			VEG_NODISCARD VEG_CPP14(constexpr) auto map_or,
			(fn, Fn&&),
			(d, meta::invoke_result_t<Fn, T>)) &&

			VEG_NOEXCEPT_IF(
					VEG_CONCEPT(nothrow_invocable<Fn, T&&>) &&
					VEG_CONCEPT(nothrow_move_constructible<meta::invoke_result_t<Fn, T>>))
					-> meta::invoke_result_t<Fn, T> {
		if (!*this) {
			return VEG_FWD(d);
		}
		return VEG_FWD(fn)(
				static_cast<Option&&>(*this).as_ref().unwrap_unchecked(unsafe));
	}

	VEG_TEMPLATE(
			(typename Fn),
			requires(
					VEG_CONCEPT(invocable<Fn>) &&
					VEG_CONCEPT(same<veg::Option<T>, meta::invoke_result_t<Fn>>)),
			VEG_NODISCARD VEG_CPP14(constexpr) auto or_else,
			(fn, Fn&&)) &&

			VEG_NOEXCEPT_IF(
					(VEG_CONCEPT(nothrow_invocable<Fn>) &&
	         VEG_CONCEPT(nothrow_copy_constructible<T>))) -> Option<T> {
		if (*this) {
			return {
					inplace,
					internal::ConvertingFn<T&&, T>{static_cast<T&&>(this->_get())},
			};
		}
		return VEG_FWD(fn)();
	}

	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) explicit
	operator bool() const VEG_NOEXCEPT {
		return this->is_engaged();
	}

	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto as_cref() const
			VEG_NOEXCEPT -> Option<meta::uncvref_t<T> const&> {
		if (*this) {
			return {some, this->_get()};
		}
		return {};
	}

	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto as_ref() & VEG_NOEXCEPT
																																-> Option<T&> {
		if (this->is_engaged()) {
			return {some, this->_get()};
		}
		return {};
	}
	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto as_ref() const
			& VEG_NOEXCEPT -> Option<T const&> {
		if (this->is_engaged()) {
			return {some, this->_get()};
		}
		return {};
	}
	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto as_ref() &&
			VEG_NOEXCEPT -> Option<T&&> {
		if (this->is_engaged()) {
			return {some, static_cast<T&&>(this->_get())};
		}
		return {};
	}
};

VEG_CPP17(

		template <typename T> Option(Some, T) -> Option<T>;

)

} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_OPTION_HPP_8NVLXES2S */
