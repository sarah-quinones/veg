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
#include "veg/uwunion.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {

template <typename T>
struct Option;

namespace option {
namespace nb {
struct some {
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
} // namespace nb
VEG_NIEBLOID(some);
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

template <typename To>
struct into_fn {
	template <typename T>
	VEG_INLINE constexpr auto operator()(T&& ref) const
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_constructible<To, T&&>)) -> To {
		return VEG_FWD(ref);
	}
};

namespace adl {
struct adl_base {};
VEG_TEMPLATE(
		(typename T, typename U),
		requires(VEG_CONCEPT(partial_eq<T, U>)),
		VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto
		operator==,
		(lhs, Option<T> const&),
		(rhs, Option<U> const&))
VEG_NOEXCEPT->bool {
	if (lhs.is_some() && rhs.is_some()) {
		return cmp::eq(
				lhs.as_ref().unwrap_unchecked(unsafe),
				rhs.as_ref().unwrap_unchecked(unsafe));
	}
	return (lhs.is_some() == rhs.is_some());
}

VEG_TEMPLATE(
		(typename T, typename U),
		requires(VEG_CONCEPT(partial_eq<T, U>)),
		VEG_INLINE VEG_CPP14(constexpr) auto
		operator!=,
		(a, Option<T> const&),
		(b, Option<U> const&))
VEG_NOEXCEPT->bool {
	return !adl::operator==(a, b);
}
} // namespace adl

template <typename T>
struct ret_none {
	VEG_INLINE VEG_CPP14(constexpr) auto operator()() const VEG_NOEXCEPT -> T {
		return none;
	}
};

} // namespace option_
} // namespace internal

template <typename T>
struct VEG_NODISCARD Option
		: private internal::_uwunion::UwunionImpl<internal::Empty, T>,
			private internal::option_::adl::adl_base {
private:
	using Base = internal::_uwunion::UwunionImpl<internal::Empty, T>;

public:
	constexpr Option() VEG_NOEXCEPT
			: Base{
						internal::_uwunion::EmplaceTag{},
						internal::UTag<usize{0}>{},
						internal::_uwunion::
								IdxConvertingFn<internal::Empty&&, internal::Empty>{
										internal::Empty{}},
						usize{0},
				} {}
	VEG_DEFAULT_CTOR_ASSIGN(Option);

	constexpr Option // NOLINT(hicpp-explicit-conversions)
			(None /*tag*/) VEG_NOEXCEPT : Option{} {}

	VEG_CONSTRAINED_MEMBER_FN(
			requires(VEG_CONCEPT(move_constructible<T>)),
			VEG_INLINE constexpr Option,
			((/*tag*/, Some), (value, T)),
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_move_constructible<T>)))
			: Base{
						internal::_uwunion::EmplaceTag{},
						internal::UTag<usize{1}>{},
						internal::_uwunion::IdxConvertingFn<T&&, T>{VEG_FWD(value)},
						usize{1},
				} {}

	VEG_TEMPLATE(
			(typename Fn),
			requires(VEG_CONCEPT(fn_once<Fn, T>)),
			VEG_INLINE constexpr Option,
			(/*tag*/, InPlace),
			(fn, Fn))
	VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_fn_once<Fn, T>))
			: Base{
						internal::_uwunion::EmplaceTag{},
						internal::UTag<usize{1}>{},
						internal::_uwunion::TaggedFn<Fn&&>{VEG_FWD(fn)},
						usize{1},
				} {}

	VEG_CPP14(constexpr)
	void reset() VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_destructible<T>)) {
		if (is_some()) {
			this->template _emplace<usize{0}>(internal::_uwunion::IdxConvertingFn<
																				internal::Empty&&,
																				internal::Empty>{internal::Empty{}});
		}
	}

	VEG_CPP14(constexpr)
	auto operator=(None /*arg*/)
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_destructible<T>)) -> Option& {
		reset();
		return *this;
	}

	VEG_CONSTRAINED_MEMBER_FN_NO_PARAM(
			requires(VEG_CONCEPT(option<T>)),
			VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto flatten,
			T,
			&&VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_move_constructible<T>))) {
		return static_cast<Option<T>&&>(*this).map_or_else(
				internal::option_::into_fn<T>{}, internal::option_::ret_none<T>{});
	}

private:
	VEG_CPP14(constexpr) auto _get() const VEG_NOEXCEPT -> T& {
		return const_cast<T&>(Base::template get_ref<1>());
	}

public:
	VEG_CONSTRAINED_MEMBER_FN_NO_PARAM(
			requires(VEG_CONCEPT(move_constructible<T>)),
			VEG_NODISCARD VEG_CPP14(constexpr) auto take,
			Option<T>,
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_move_constructible<T>))) {
		if (is_some()) {
			Option<T> val{
					inplace,
					internal::ConvertingFn<T&&, T>{static_cast<T&&>(this->_get())},
			};
			reset();
			return val;
		}
		return none;
	}

	VEG_CONSTRAINED_MEMBER_FN(
			requires(VEG_CONCEPT(move_constructible<T>)),
			VEG_NODISCARD VEG_CPP14(constexpr) auto unwrap_unchecked,
			((/*tag*/, Unsafe)),
			&&VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_move_constructible<T>))->T) {
		meta::unreachable_if(is_none());
		return static_cast<T&&>(this->_get());
	}

	VEG_CONSTRAINED_MEMBER_FN_NO_PARAM(
			requires(VEG_CONCEPT(move_constructible<T>)),
			VEG_NODISCARD VEG_CPP14(constexpr) auto unwrap,
			T,
			&&VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_move_constructible<T>))) {
		VEG_INTERNAL_ASSERT_PRECONDITION(is_some());
		return static_cast<T&&>(this->_get());
	}

	VEG_TEMPLATE(
			(typename Fn),
			requires(
					VEG_CONCEPT(move_constructible<T>) &&
					VEG_CONCEPT(fn_once<Fn, bool, Ref<T>>)),
			VEG_NODISCARD VEG_CPP14(constexpr) auto filter,
			(fn, Fn)) &&

			VEG_NOEXCEPT_IF(
					(VEG_CONCEPT(nothrow_fn_once<Fn, bool, Ref<T>>) &&
	         VEG_CONCEPT(nothrow_move_constructible<T>))) -> Option<T> {
		auto&& self = static_cast<Option<T>&&>(*this);
		if (self.is_some()) {
			if (VEG_FWD(fn)(Ref<T>{AsRef{}, this->_get()})) {
				return {
						inplace,
						internal::ConvertingFn<T&&, T>{static_cast<T&&>(this->_get())},
				};
			}
		}
		return none;
	}

	VEG_TEMPLATE(
			(typename U = T),
			requires(VEG_CONCEPT(partial_eq<T, U>)),
			VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto contains,
			(val, Ref<U>))
	const VEG_NOEXCEPT->bool {
		if (is_none()) {
			return false;
		}
		return cmp::eq(Ref<T>{AsRef{}, _get()}, val);
	}

	VEG_TEMPLATE(
			typename Fn,
			requires(VEG_CONCEPT(fn_once<Fn, T>)),
			VEG_CPP14(constexpr) auto emplace,
			(fn, Fn)) &
			VEG_NOEXCEPT_IF(
					VEG_CONCEPT(nothrow_destructible<T>) &&
					VEG_CONCEPT(nothrow_fn_once<Fn, T>)) -> T& {
		this->template _emplace<usize{1}>(
				internal::_uwunion::TaggedFn<Fn&&>{VEG_FWD(fn)});
		return this->_get();
	}

	VEG_TEMPLATE(
			(typename Fn, typename Ret = meta::invoke_result_t<Fn, T>),
			requires(VEG_CONCEPT(fn_once<Fn, Ret, T>)),
			VEG_NODISCARD VEG_CPP14(constexpr) auto map,
			(fn, Fn)) &&

			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_fn_once<Fn, Ret, T>)) -> Option<Ret> {
		if (is_some()) {
			return Option<Ret>{
					inplace,
					internal::WithArg<Fn&&, T&&>{
							VEG_FWD(fn), static_cast<T&&>(this->_get())},
			};
		}
		return none;
	}

	VEG_TEMPLATE(
			(typename Fn, typename Ret = meta::invoke_result_t<Fn, T>),
			requires(
					VEG_CONCEPT(fn_once<Fn, Ret, T>) && //
					VEG_CONCEPT(option<Ret>)),
			VEG_NODISCARD VEG_CPP14(constexpr) auto and_then,
			(fn, Fn)) &&

			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_fn_once<Fn, Ret, T>)) -> Ret {
		if (is_some()) {
			return VEG_FWD(fn)(static_cast<T&&>(this->_get()));
		}
		return none;
	}

	VEG_TEMPLATE(
			(typename Fn, typename D, typename Ret = meta::invoke_result_t<Fn, T>),
			requires(
					VEG_CONCEPT(fn_once<Fn, Ret, T>) && //
					VEG_CONCEPT(fn_once<D, Ret>)),
			VEG_NODISCARD VEG_CPP14(constexpr) auto map_or_else,
			(fn, Fn),
			(d, D)) &&

			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_fn_once<Fn, Ret>)) -> Ret {
		if (is_some()) {
			return VEG_FWD(fn)(static_cast<T&&>(this->_get()));
		}
		return VEG_FWD(d)();
	}

	VEG_TEMPLATE(
			(typename Fn, typename Ret = meta::invoke_result_t<Fn, T>),
			requires(
					VEG_CONCEPT(fn_once<Fn, Ret, T>) &&
					VEG_CONCEPT(move_constructible<Ret>)),
			VEG_NODISCARD VEG_CPP14(constexpr) auto map_or,
			(fn, Fn),
			(d, Ret)) &&

			VEG_NOEXCEPT_IF(
					VEG_CONCEPT(nothrow_fn_once<Fn, Ret, T>) &&
					VEG_CONCEPT(nothrow_move_constructible<Ret>)) -> Ret {
		if (is_none()) {
			return VEG_FWD(d);
		}
		return VEG_FWD(fn)(static_cast<T&&>(this->_get()));
	}

	VEG_TEMPLATE(
			(typename Fn),
			requires(
					VEG_CONCEPT(fn_once<Fn, Option<T>>) &&
					VEG_CONCEPT(move_constructible<T>)),
			VEG_NODISCARD VEG_CPP14(constexpr) auto or_else,
			(fn, Fn)) &&

			VEG_NOEXCEPT_IF(
					(VEG_CONCEPT(nothrow_fn_once<Fn, Option<T>>) &&
	         VEG_CONCEPT(nothrow_move_constructible<T>))) -> Option<T> {
		if (is_some()) {
			return {
					inplace,
					internal::ConvertingFn<T&&, T>{static_cast<T&&>(this->_get())},
			};
		}
		return VEG_FWD(fn)();
	}

	VEG_NODISCARD VEG_INLINE constexpr auto is_some() const VEG_NOEXCEPT -> bool {
		return usize(Base::index()) != 0;
	}
	VEG_NODISCARD VEG_INLINE constexpr auto is_none() const VEG_NOEXCEPT -> bool {
		return !is_some();
	}

	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto as_cref() const
			VEG_NOEXCEPT -> Option<meta::uncvref_t<T> const&> {
		if (is_some()) {
			return {some, this->_get()};
		}
		return {};
	}

	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto as_mut() VEG_NOEXCEPT
			-> Option<RefMut<T>> {
		if (is_some()) {
			return {some, veg::mut(this->_get())};
		}
		return {};
	}
	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto as_ref() const
			& VEG_NOEXCEPT -> Option<Ref<T>> {
		if (is_some()) {
			return {some, ref(this->_get())};
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
