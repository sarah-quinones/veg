#ifndef VEG_CURRY_HPP_KV8QZWDES
#define VEG_CURRY_HPP_KV8QZWDES

#include "veg/type_traits/invocable.hpp"
#include "veg/type_traits/constructible.hpp"
#include "veg/internal/tuple_generic.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {

namespace internal {
namespace _fn {

template <
		typename Fn,
		typename... StoredArgs,
		typename... Args,
		usize... Is,
		typename Ret = meta::invoke_result_t<Fn, Args..., StoredArgs...>>
VEG_INLINE static constexpr auto call_bound_back_once(
		Fn&& fn,
		tuple::IndexedTuple<meta::index_sequence<Is...>, StoredArgs...>&& stored,
		Args&&... args)
		VEG_NOEXCEPT_IF(
				VEG_CONCEPT(nothrow_fn_once<Fn, Ret, Args..., StoredArgs...>)) -> Ret {
	return VEG_FWD(fn)(
			VEG_FWD(args)...,
			static_cast<StoredArgs&&>(
					static_cast<tuple::TupleLeaf<Is, StoredArgs>&>(stored)
							.leaf_get())...);
}
template <
		typename Fn,
		typename... StoredArgs,
		typename... Args,
		usize... Is,
		typename Ret = meta::invoke_result_t<Fn, StoredArgs..., Args...>>
VEG_INLINE static constexpr auto call_bound_front_once(
		Fn&& fn,
		tuple::IndexedTuple<meta::index_sequence<Is...>, StoredArgs...>&& stored,
		Args&&... args)
		VEG_NOEXCEPT_IF(
				VEG_CONCEPT(nothrow_fn_once<Fn, Ret, StoredArgs..., Args...>)) -> Ret {
	return VEG_FWD(fn)(
			static_cast<StoredArgs&&>(
					static_cast<tuple::TupleLeaf<Is, StoredArgs>&>(stored).leaf_get())...,
			VEG_FWD(args)...);
}

template <
		typename Fn,
		typename... StoredArgs,
		typename... Args,
		usize... Is,
		typename Ret = meta::invoke_result_t<Fn&, Args..., RefMut<StoredArgs>...>>
VEG_INLINE static constexpr auto call_bound_back_mut(
		Fn& fn,
		tuple::IndexedTuple<meta::index_sequence<Is...>, StoredArgs...>& stored,
		Args&&... args)
		VEG_NOEXCEPT_IF(VEG_CONCEPT(
				nothrow_fn_mut<Fn, Ret, Args..., RefMut<StoredArgs>...>)) -> Ret {
	return fn(
			VEG_FWD(args)...,
			RefMut<StoredArgs>{
					AsMut{},
					static_cast<tuple::TupleLeaf<Is, StoredArgs>&>(stored).leaf_get(),
			}...);
}
template <
		typename Fn,
		typename... StoredArgs,
		typename... Args,
		usize... Is,
		typename Ret = meta::invoke_result_t<Fn&, RefMut<StoredArgs>..., Args...>>
VEG_INLINE static constexpr auto call_bound_front_mut(
		Fn& fn,
		tuple::IndexedTuple<meta::index_sequence<Is...>, StoredArgs...>& stored,
		Args&&... args)
		VEG_NOEXCEPT_IF(VEG_CONCEPT(
				nothrow_fn_mut<Fn, Ret, RefMut<StoredArgs>..., Args...>)) -> Ret {
	return fn(
			RefMut<StoredArgs>{
					AsMut{},
					static_cast<tuple::TupleLeaf<Is, StoredArgs>&>(stored).leaf_get(),
			}...,
			VEG_FWD(args)...);
}

template <
		typename Fn,
		typename... StoredArgs,
		typename... Args,
		usize... Is,
		typename Ret =
				meta::invoke_result_t<Fn const&, Args..., Ref<StoredArgs>...>>
VEG_INLINE static constexpr auto call_bound_back(
		Fn const& fn,
		tuple::IndexedTuple<meta::index_sequence<Is...>, StoredArgs...> const&
				stored,
		Args&&... args)
		VEG_NOEXCEPT_IF(
				VEG_CONCEPT(nothrow_fn<Fn, Ret, Args..., Ref<StoredArgs>...>)) -> Ret {
	return fn(
			VEG_FWD(args)...,
			Ref<StoredArgs>{
					AsRef{},
					static_cast<tuple::TupleLeaf<Is, StoredArgs> const&>(stored)
							.leaf_get(),
			}...);
}
template <
		typename Fn,
		typename... StoredArgs,
		typename... Args,
		usize... Is,
		typename Ret =
				meta::invoke_result_t<Fn const&, Ref<StoredArgs>..., Args...>>
VEG_INLINE static constexpr auto call_bound_front(
		Fn const& fn,
		tuple::IndexedTuple<meta::index_sequence<Is...>, StoredArgs...> const&
				stored,
		Args&&... args)
		VEG_NOEXCEPT_IF(
				VEG_CONCEPT(nothrow_fn<Fn, Ret, Ref<StoredArgs>..., Args...>)) -> Ret {
	return fn(
			Ref<StoredArgs>{
					AsRef{},
					static_cast<tuple::TupleLeaf<Is, StoredArgs> const&>(stored)
							.leaf_get(),
			}...,
			VEG_FWD(args)...);
}

template <
		typename Fn,
		typename... StoredArgs,
		typename... Args,
		usize... Is,
		typename Ret = meta::invoke_result_t<Fn const&, Args..., StoredArgs...>>
VEG_INLINE static constexpr auto call_bound_back_copy(
		Fn const& fn,
		tuple::IndexedTuple<meta::index_sequence<Is...>, StoredArgs...> const&
				stored,
		Args&&... args)
		VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_fn<Fn, Ret, Args..., StoredArgs...>))
				-> Ret {
	return fn(
			VEG_FWD(args)...,
			static_cast<StoredArgs>(
					static_cast<tuple::TupleLeaf<Is, StoredArgs> const&>(stored)
							.leaf_get())...);
}
template <
		typename Fn,
		typename... StoredArgs,
		typename... Args,
		usize... Is,
		typename Ret = meta::invoke_result_t<Fn const&, StoredArgs..., Args...>>
VEG_INLINE static constexpr auto call_bound_front_copy(
		Fn const& fn,
		tuple::IndexedTuple<meta::index_sequence<Is...>, StoredArgs...> const&
				stored,
		Args&&... args)
		VEG_NOEXCEPT_IF(
				VEG_CONCEPT(nothrow_fn<Fn, Ret, Ref<StoredArgs>..., Args...>)) -> Ret {
	return fn(
			static_cast<StoredArgs>(
					static_cast<tuple::TupleLeaf<Is, StoredArgs> const&>(stored)
							.leaf_get())...,
			VEG_FWD(args)...);
}

} // namespace _fn
} // namespace internal

namespace fn {

template <typename Fn, typename... StoredArgs>
struct BindBackOnce {
	Fn fn;
	Tuple<StoredArgs...> stored_args;

	VEG_TEMPLATE(
			(typename... Args,
	     typename Ret = meta::invoke_result_t<Fn, Args..., StoredArgs...>),
			requires(VEG_CONCEPT(fn_once<Fn, Ret, Args..., StoredArgs...>)),
			VEG_INLINE VEG_CPP14(constexpr) auto
			operator(),
			(... args, Args&&)) &&
			VEG_NOEXCEPT_IF(VEG_CONCEPT(
					nothrow_fn_once<Fn, Ret, Args..., StoredArgs...>)) -> Ret {
		return internal::_fn::call_bound_back_once(
				VEG_FWD(fn), VEG_FWD(stored_args), VEG_FWD(args)...);
	}
};

template <typename Fn, typename... StoredArgs>
struct BindFrontOnce {
	Fn fn;
	Tuple<StoredArgs...> stored_args;

	VEG_TEMPLATE(
			(typename... Args,
	     typename Ret = meta::invoke_result_t<Fn, StoredArgs..., Args...>),
			requires(VEG_CONCEPT(fn_once<Fn, Ret, StoredArgs..., Args...>)),
			VEG_INLINE VEG_CPP14(constexpr) auto
			operator(),
			(... args, Args&&)) &&
			VEG_NOEXCEPT_IF(VEG_CONCEPT(
					nothrow_fn_once<Fn, Ret, StoredArgs..., Args...>)) -> Ret {
		return internal::_fn::call_bound_front_once(
				VEG_FWD(fn), VEG_FWD(stored_args), VEG_FWD(args)...);
	}
};

template <typename Fn, typename... StoredArgs>
struct BindBackMut {
	Fn fn;
	Tuple<StoredArgs...> stored_args;

	VEG_TEMPLATE(
			(typename... Args,
	     typename Ret =
	         meta::invoke_result_t<Fn&, Args..., RefMut<StoredArgs>...>),
			requires(VEG_CONCEPT(fn_mut<Fn, Ret, Args..., RefMut<StoredArgs>...>)),
			VEG_INLINE VEG_CPP14(constexpr) auto
			operator(),
			(... args, Args&&))
	VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_fn_mut<Fn, Ret, Args..., RefMut<StoredArgs>...>))
			->Ret {
		return internal::_fn::call_bound_back_mut(
				fn, stored_args, VEG_FWD(args)...);
	}
};

template <typename Fn, typename... StoredArgs>
struct BindFrontMut {
	Fn fn;
	Tuple<StoredArgs...> stored_args;

	VEG_TEMPLATE(
			(typename... Args,
	     typename Ret =
	         meta::invoke_result_t<Fn&, RefMut<StoredArgs>..., Args...>),
			requires(VEG_CONCEPT(fn_mut<Fn, Ret, RefMut<StoredArgs>..., Args...>)),
			VEG_INLINE VEG_CPP14(constexpr) auto
			operator(),
			(... args, Args&&))
	VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_fn_mut<Fn, Ret, RefMut<StoredArgs>..., Args...>))
			->Ret {
		return internal::_fn::call_bound_front_mut(
				fn, stored_args, VEG_FWD(args)...);
	}
};

template <typename Fn, typename... StoredArgs>
struct BindBack {
	Fn fn;
	Tuple<StoredArgs...> stored_args;

	VEG_TEMPLATE(
			(typename... Args,
	     typename Ret =
	         meta::invoke_result_t<Fn const&, Args..., Ref<StoredArgs>...>),
			requires(VEG_CONCEPT(fn<Fn, Ret, Args..., Ref<StoredArgs>...>)),
			VEG_INLINE constexpr auto
			operator(),
			(... args, Args&&))
	const VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_fn<Fn, Ret, Args..., Ref<StoredArgs>...>))
			->Ret {
		return internal::_fn::call_bound_back(fn, stored_args, VEG_FWD(args)...);
	}
};

template <typename Fn, typename... StoredArgs>
struct BindFront {
	Fn fn;
	Tuple<StoredArgs...> stored_args;

	VEG_TEMPLATE(
			(typename... Args,
	     typename Ret =
	         meta::invoke_result_t<Fn const&, Ref<StoredArgs>..., Args...>),
			requires(VEG_CONCEPT(fn<Fn, Ret, Ref<StoredArgs>..., Args...>)),
			VEG_INLINE constexpr auto
			operator(),
			(... args, Args&&))
	const VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_fn<Fn, Ret, Ref<StoredArgs>..., Args...>))
			->Ret {
		return internal::_fn::call_bound_front(fn, stored_args, VEG_FWD(args)...);
	}
};

template <typename Fn, typename... StoredArgs>
struct BindBackCopy {
	Fn fn;
	Tuple<StoredArgs...> stored_args;

	VEG_TEMPLATE(
			(typename... Args,
	     typename Ret = meta::invoke_result_t<Fn const&, Args..., StoredArgs...>),
			requires(VEG_CONCEPT(fn<Fn, Ret, Args..., StoredArgs...>)),
			VEG_INLINE constexpr auto
			operator(),
			(... args, Args&&))
	const VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_fn<Fn, Ret, Args..., StoredArgs...>))
			->Ret {
		return internal::_fn::call_bound_back_copy(
				fn, stored_args, VEG_FWD(args)...);
	}
};
template <typename Fn, typename... StoredArgs>
struct BindFrontCopy {
	Fn fn;
	Tuple<StoredArgs...> stored_args;

	VEG_TEMPLATE(
			(typename... Args,
	     typename Ret = meta::invoke_result_t<Fn const&, StoredArgs..., Args...>),
			requires(VEG_CONCEPT(fn<Fn, Ret, StoredArgs..., Args...>)),
			VEG_INLINE constexpr auto
			operator(),
			(... args, Args&&))
	const VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_fn<Fn, Ret, StoredArgs..., Args...>))
			->Ret {
		return internal::_fn::call_bound_front_copy(
				fn, stored_args, VEG_FWD(args)...);
	}
};

namespace nb {
struct bind_back_once {
	VEG_TEMPLATE(
			(typename Fn, typename... Args),
			requires(
					VEG_CONCEPT(movable<Fn>) && VEG_ALL_OF(VEG_CONCEPT(movable<Args>))),
			VEG_INLINE constexpr auto
			operator(),
			(fn, Fn),
			(... args, Args))
	const VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_movable<Fn>) &&
			VEG_ALL_OF(VEG_CONCEPT(nothrow_movable<Args>)))
			->BindBackOnce<Fn, Args...> {
		return {
				VEG_FWD(fn),
				Tuple<Args...>{
						Direct{},
						VEG_FWD(args)...,
				},
		};
	}
};
struct bind_front_once {
	VEG_TEMPLATE(
			(typename Fn, typename... Args),
			requires(
					VEG_CONCEPT(movable<Fn>) && VEG_ALL_OF(VEG_CONCEPT(movable<Args>))),
			VEG_INLINE constexpr auto
			operator(),
			(fn, Fn),
			(... args, Args))
	const VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_movable<Fn>) &&
			VEG_ALL_OF(VEG_CONCEPT(nothrow_movable<Args>)))
			->BindFrontOnce<Fn, Args...> {
		return {
				VEG_FWD(fn),
				{
						Direct{},
						VEG_FWD(args)...,
				},
		};
	}
};

struct bind_back_mut {
	VEG_TEMPLATE(
			(typename Fn, typename... Args),
			requires(
					VEG_CONCEPT(movable<Fn>) && VEG_ALL_OF(VEG_CONCEPT(movable<Args>))),
			VEG_INLINE constexpr auto
			operator(),
			(fn, Fn),
			(... args, Args))
	const VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_movable<Fn>) &&
			VEG_ALL_OF(VEG_CONCEPT(nothrow_movable<Args>)))
			->BindBackMut<Fn, Args...> {
		return {
				VEG_FWD(fn),
				Tuple<Args...>{
						Direct{},
						VEG_FWD(args)...,
				},
		};
	}
};
struct bind_front_mut {
	VEG_TEMPLATE(
			(typename Fn, typename... Args),
			requires(
					VEG_CONCEPT(movable<Fn>) && VEG_ALL_OF(VEG_CONCEPT(movable<Args>))),
			VEG_INLINE constexpr auto
			operator(),
			(fn, Fn),
			(... args, Args))
	const VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_movable<Fn>) &&
			VEG_ALL_OF(VEG_CONCEPT(nothrow_movable<Args>)))
			->BindFrontMut<Fn, Args...> {
		return {
				VEG_FWD(fn),
				{
						Direct{},
						VEG_FWD(args)...,
				},
		};
	}
};

struct bind_back {
	VEG_TEMPLATE(
			(typename Fn, typename... Args),
			requires(
					VEG_CONCEPT(movable<Fn>) && VEG_ALL_OF(VEG_CONCEPT(movable<Args>))),
			VEG_INLINE constexpr auto
			operator(),
			(fn, Fn),
			(... args, Args))
	const VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_movable<Fn>) &&
			VEG_ALL_OF(VEG_CONCEPT(nothrow_movable<Args>)))
			->BindBack<Fn, Args...> {
		return {
				VEG_FWD(fn),
				Tuple<Args...>{
						Direct{},
						VEG_FWD(args)...,
				},
		};
	}
};
struct bind_front {
	VEG_TEMPLATE(
			(typename Fn, typename... Args),
			requires(
					VEG_CONCEPT(movable<Fn>) && VEG_ALL_OF(VEG_CONCEPT(movable<Args>))),
			VEG_INLINE constexpr auto
			operator(),
			(fn, Fn),
			(... args, Args))
	const VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_movable<Fn>) &&
			VEG_ALL_OF(VEG_CONCEPT(nothrow_movable<Args>)))
			->BindFront<Fn, Args...> {
		return {
				VEG_FWD(fn),
				{
						Direct{},
						VEG_FWD(args)...,
				},
		};
	}
};

struct bind_back_copy {
	VEG_TEMPLATE(
			(typename Fn, typename... Args),
			requires(
					VEG_CONCEPT(movable<Fn>) && VEG_ALL_OF(VEG_CONCEPT(movable<Args>))),
			VEG_INLINE constexpr auto
			operator(),
			(fn, Fn),
			(... args, Args))
	const VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_movable<Fn>) &&
			VEG_ALL_OF(VEG_CONCEPT(nothrow_movable<Args>)))
			->BindBackCopy<Fn, Args...> {
		return {
				VEG_FWD(fn),
				{
						Direct{},
						VEG_FWD(args)...,
				},
		};
	}
};
struct bind_front_copy {
	VEG_TEMPLATE(
			(typename Fn, typename... Args),
			requires(
					VEG_CONCEPT(movable<Fn>) && VEG_ALL_OF(VEG_CONCEPT(movable<Args>))),
			VEG_INLINE constexpr auto
			operator(),
			(fn, Fn),
			(... args, Args))
	const VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_movable<Fn>) &&
			VEG_ALL_OF(VEG_CONCEPT(nothrow_movable<Args>)))
			->BindFrontCopy<Fn, Args...> {
		return {
				VEG_FWD(fn),
				{
						Direct{},
						VEG_FWD(args)...,
				},
		};
	}
};
} // namespace nb
VEG_NIEBLOID(bind_back_once);
VEG_NIEBLOID(bind_front_once);
VEG_NIEBLOID(bind_back_mut);
VEG_NIEBLOID(bind_front_mut);
VEG_NIEBLOID(bind_back);
VEG_NIEBLOID(bind_front);
VEG_NIEBLOID(bind_back_copy);
VEG_NIEBLOID(bind_front_copy);
} // namespace fn
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_CURRY_HPP_KV8QZWDES */
