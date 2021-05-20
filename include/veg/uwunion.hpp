#ifndef VEG_UWUNION_HPP_OHC4GK5JS
#define VEG_UWUNION_HPP_OHC4GK5JS

#include "veg/util/assert.hpp"
#include "veg/memory/address.hpp"
#include "veg/memory/placement.hpp"
#include "veg/internal/delete_special_members.hpp"
#include "veg/type_traits/constructible.hpp"
#include "veg/type_traits/assignable.hpp"
#include "veg/internal/integer_seq.hpp"
#include "veg/internal/visit.hpp"
#include "veg/internal/fix_index.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
namespace internal {
namespace _uwunion {

template <typename T>
struct Storage {
	T inner;
};

struct TrivialTag;
struct NonTrivialTag;

template <bool _, typename... Ts>
union Uwunion_;

template <typename... Ts>
using Uwunion = Uwunion_<false, Ts...>;
template <usize I>
struct UwunionGetImpl;

template <>
union Uwunion_<false> {
	Empty _{};
};

#define VEG_TYPE_DECL(_, I) , typename __VEG_PP_CAT(T, I)
#define VEG_TYPE_PUT(_, I) , __VEG_PP_CAT(T, I)
#define VEG_UWUNION_HEAD(_, I)                                                 \
	Storage<__VEG_PP_CAT(T, I)> __VEG_PP_CAT(head, I);
#define VEG_UWUNION_HEAD_CTOR(_, I)                                            \
	template <usize J, typename Fn>                                              \
	VEG_INLINE constexpr Uwunion_(UTag<I> /*unused*/, UTag<J>, Fn fn)            \
			VEG_NOEXCEPT_LIKE(VEG_FWD(fn)(UTag<J>{}))                                \
			: __VEG_PP_CAT(head, I){VEG_FWD(fn)(UTag<J>{})} {}

#define VEG_UWUNION_SPEC(Tuple)                                                \
	template <bool _ __VEG_PP_TUPLE_FOR_EACH(VEG_TYPE_DECL, _, Tuple)>           \
	union Uwunion_<_ __VEG_PP_TUPLE_FOR_EACH(VEG_TYPE_PUT, _, Tuple)> {          \
		__VEG_PP_TUPLE_FOR_EACH(VEG_UWUNION_HEAD, _, Tuple)                        \
		__VEG_PP_TUPLE_FOR_EACH(VEG_UWUNION_HEAD_CTOR, _, Tuple)                   \
	};                                                                           \
	template <>                                                                  \
	struct UwunionGetImpl<__VEG_PP_TUPLE_SIZE(Tuple)> {                          \
		template <typename... Ts>                                                  \
		VEG_INLINE static constexpr auto get(Uwunion<Ts...> const& u) VEG_NOEXCEPT \
				-> ith<__VEG_PP_TUPLE_SIZE(Tuple), Ts...> const& {                     \
			return u.__VEG_PP_CAT(head, __VEG_PP_TUPLE_SIZE(Tuple)).inner;           \
		}                                                                          \
	};

#define VEG_UWUNION_TPL(Tuple)

template <>
struct UwunionGetImpl<0> {
	template <typename... Ts>
	VEG_INLINE static constexpr auto get(Uwunion<Ts...> const& u) VEG_NOEXCEPT
			-> ith<0, Ts...> const& {
		return u.head0.inner;
	}
};

VEG_UWUNION_SPEC(__VEG_PP_MAKE_TUPLE(3));
VEG_UWUNION_SPEC(__VEG_PP_MAKE_TUPLE(2));
VEG_UWUNION_SPEC(__VEG_PP_MAKE_TUPLE(1));
#define VEG_TUPLE __VEG_PP_MAKE_TUPLE(4)

template <
		bool _ __VEG_PP_TUPLE_FOR_EACH(VEG_TYPE_DECL, _, VEG_TUPLE),
		typename... Ts>
union Uwunion_<_ __VEG_PP_TUPLE_FOR_EACH(VEG_TYPE_PUT, _, VEG_TUPLE), Ts...> {
	Uwunion<Ts...> tail;
	__VEG_PP_TUPLE_FOR_EACH(VEG_UWUNION_HEAD, _, VEG_TUPLE)
	__VEG_PP_TUPLE_FOR_EACH(VEG_UWUNION_HEAD_CTOR, _, VEG_TUPLE)
	template <usize I, usize J, typename Fn>
	VEG_INLINE constexpr Uwunion_(UTag<I> /*unused*/, UTag<J> tag, Fn fn)
			VEG_NOEXCEPT_LIKE(VEG_FWD(fn)(UTag<J>{}))
			: tail{UTag<I - __VEG_PP_TUPLE_SIZE(VEG_TUPLE)>{}, tag, VEG_FWD(fn)} {}
};
template <usize I>
struct UwunionGetImpl {
	template <typename... Ts>
	VEG_INLINE static constexpr auto get(Uwunion<Ts...> const& u) VEG_NOEXCEPT
			-> ith<I, Ts...> const& {
		return UwunionGetImpl<I - __VEG_PP_TUPLE_SIZE(VEG_TUPLE)>::get(u.tail);
	}
};

#undef VEG_TUPLE
#undef VEG_UWUNION_TPL
#undef VEG_UWUNION_SPEC
#undef VEG_UWUNION_DEF
#undef VEG_UWUNION_HEAD_CTOR
#undef VEG_UWUNION_HEAD
#undef VEG_TYPE_PUT
#undef VEG_TYPE_DECL

enum non_trivial_kind_e {
	needs_copy_move,
	needs_dtor,
	needs_double_storage,
};
template <non_trivial_kind_e Kind, typename ISeq, typename... Ts>
struct NonTrivialTaggedImpl;

template <typename U>
struct UwunionGetter;

template <typename U>
struct UwunionGetter<U const&> {
	U const& self;
	template <usize I>
	VEG_INLINE constexpr auto operator()(UTag<I> /*unused*/) const noexcept ->
			typename U::template Ith<I> const& {
		return self.template get_ref<I>();
	}
};
template <typename U>
struct UwunionGetter<U&> {
	U& self;
	template <usize I>
	VEG_INLINE constexpr auto operator()(UTag<I> /*unused*/) const noexcept ->
			typename U::template Ith<I>& {
		return const_cast<typename U::template Ith<I>&>(self.template get_ref<I>());
	}
};
template <typename U>
struct UwunionGetter<U&&> {
	U& self;
	template <usize I>
	VEG_INLINE constexpr auto operator()(UTag<I> /*unused*/) const noexcept ->
			typename U::template Ith<I>&& {
		return const_cast<typename U::template Ith<I>&&>(
				static_cast<U&&>(self).template get_ref<I>());
	}
};

struct EmplaceTag {};

template <typename Target, typename Fn>
struct EmplaceWrapper {
	Fn const fn;
	usize i;
	template <usize I>
	VEG_INLINE constexpr auto
	operator()(UTag<I> tag) const&& VEG_NOEXCEPT_LIKE(VEG_FWD(fn)(UTag<I>{}))
			-> Target {
		return {EmplaceTag{}, tag, VEG_FWD(fn), i};
	}
};

template <typename U, typename Fn>
struct CtorFn {
	U& self;
	Fn fn;

	template <usize I>
			VEG_INLINE VEG_CPP20(constexpr) void operator()(UTag<I> tag) &&
			VEG_NOEXCEPT_LIKE(VEG_FWD(fn)(tag)) {
		mem::construct_at(mem::addressof(self), tag, tag, VEG_FWD(fn));
	};
};

template <typename U, typename Fn>
struct AssignFn {
	U& self;
	Fn fn;

	template <usize I>
			VEG_INLINE VEG_CPP14(constexpr) void operator()(UTag<I> tag) &&
			VEG_NOEXCEPT_LIKE(VEG_FWD(fn)(UTag<I>{})) {
		const_cast<typename U::template Ith<I>&>(self.template get_ref<I>()) =
				VEG_FWD(fn)(tag);
	};
};

template <typename U>
struct DropFn {
	U& self;

	template <usize I>
	VEG_INLINE VEG_CPP20(constexpr) void
	operator()(UTag<I> /*tag*/) const noexcept {
		mem::destroy_at(mem::addressof(
				const_cast<typename U::template Ith<I>&>(self.template get_ref<I>())));
	};
};

template <typename ISeq, typename... Ts>
struct TrivialTaggedImpl;

template <usize... Is, typename... Ts>
struct TrivialTaggedImpl<meta::index_sequence<Is...>, Ts...> {
	template <usize I>
	using Ith = ith<I, Ts...>;
	using TagType = meta::conditional_t<sizeof...(Ts) < 256U, u8, usize>;

	static_assert(VEG_ALL_OF(VEG_CONCEPT(trivially_copyable<Storage<Ts>>)), ".");
	union {
		Empty _;
		Uwunion<Ts...> inner;
	};
	TagType tag;

	template <usize I, typename Fn>
	VEG_INLINE constexpr TrivialTaggedImpl(
			EmplaceTag /*tag*/, UTag<I> tag, Fn fn, usize i = I)
			VEG_NOEXCEPT_LIKE(VEG_FWD(fn)(UTag<I>{}))
			: inner{tag, tag, VEG_FWD(fn)}, tag(TagType(i)) {}

	template <usize I, typename Fn>
	VEG_INLINE VEG_CPP20(constexpr) void _emplace(Fn fn)
			VEG_NOEXCEPT_LIKE(VEG_FWD(fn)(UTag<I>{})) {
		uninit_emplace<I>(VEG_FWD(fn));
		tag = I;
	}

	template <usize I, typename Fn>
	VEG_INLINE VEG_CPP20(constexpr) void uninit_emplace(Fn fn)
			VEG_NOEXCEPT_LIKE(VEG_FWD(fn)(UTag<I>{})) {
		mem::construct_at(mem::addressof(inner), UTag<I>{}, UTag<I>{}, VEG_FWD(fn));
	}

	template <usize I>
			constexpr auto get_ref() const & VEG_NOEXCEPT -> Ith<I> const& {
		return (void)meta::unreachable_if(tag != I), UwunionGetImpl<I>::get(inner);
	}

	VEG_INLINE constexpr auto index() const -> usize { return tag; }
};

template <typename Base, usize N, bool NoExcept, typename Fn>
VEG_INLINE constexpr auto make(Fn fn, usize tag) VEG_NOEXCEPT_IF(NoExcept)
		-> Base {
	return internal::visit<Base, NoExcept, N>(
			tag, EmplaceWrapper<Base, Fn>{VEG_FWD(fn), tag});
}

template <bool NeedsDtor, typename... Ts>
struct TaggedDtorBase;

#define VEG_TAGGED_UWUNION_DTOR_true                                           \
	VEG_INLINE VEG_CPP14(constexpr) void destroy() VEG_NOEXCEPT {                \
		internal::visit<void, true, sizeof...(Ts)>(                                \
				tag, DropFn<TaggedDtorBase>{{*this}});                                 \
	}                                                                            \
	VEG_INLINE VEG_CPP20(constexpr) ~TaggedDtorBase() { destroy(); }

#define VEG_TAGGED_UWUNION_DTOR_false                                          \
	VEG_INLINE VEG_CPP14(constexpr) void destroy() VEG_NOEXCEPT {}

#define VEG_TAGGED_UWUNION_MOVE_false(Class)                                   \
	Class(Class&&) = default; /* NOLINT */
#define VEG_TAGGED_UWUNION_COPY_false(Class) Class(Class const&) = default;

#if __cplusplus >= 201703L

#define VEG_TAGGED_UWUNION_MOVE_true(Class)                                    \
	VEG_INLINE                                                                   \
	constexpr Class(Class&& rhs)                                                 \
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Ts>))) \
			: inner{_uwunion::make<                                                  \
						decltype(inner),                                                   \
						sizeof...(Ts),                                                     \
						VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Ts>))>(          \
						UwunionGetter<decltype(inner)&&>{rhs.inner}, rhs.inner.tag)} {}

#define VEG_TAGGED_UWUNION_COPY_true(Class)                                    \
	VEG_INLINE                                                                   \
	constexpr Class(Class const& rhs)                                            \
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_copy_constructible<Ts>))) \
			: inner{_uwunion::make<                                                  \
						decltype(inner),                                                   \
						sizeof...(Ts),                                                     \
						VEG_ALL_OF(VEG_CONCEPT(nothrow_copy_constructible<Ts>))>(          \
						UwunionGetter<decltype(inner) const&>{rhs.inner},                  \
						rhs.inner.tag)} {}

#else

#define VEG_TAGGED_UWUNION_MOVE_true(Class)                                    \
	VEG_INLINE                                                                   \
	Class(Class&& rhs) /* NOLINT */                                              \
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Ts>))) \
			: inner{} {                                                              \
		using Fn = UwunionGetter<decltype(inner)&&>;                               \
		inner.tag = rhs.inner.tag;                                                 \
		internal::visit<                                                           \
				void,                                                                  \
				VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Ts>)),               \
				sizeof...(Ts)>(                                                        \
				this->inner.tag,                                                       \
				CtorFn<decltype(inner.inner), Fn>{inner.inner, Fn{rhs.inner}});        \
	}

#define VEG_TAGGED_UWUNION_COPY_true(Class)                                    \
	VEG_INLINE                                                                   \
	Class(Class const& rhs)                                                      \
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_copy_constructible<Ts>))) \
			: inner{} {                                                              \
		using Fn = UwunionGetter<decltype(inner) const&>;                          \
		inner.tag = rhs.inner.tag;                                                 \
		internal::visit<                                                           \
				void,                                                                  \
				VEG_ALL_OF(VEG_CONCEPT(nothrow_copy_constructible<Ts>)),               \
				sizeof...(Ts)>(                                                        \
				this->inner.tag,                                                       \
				CtorFn<decltype(inner.inner), Fn>{inner.inner, Fn{rhs.inner}});        \
	}

#endif

#define VEG_TAGGED_UWUNION_DEF(NeedsDtor)                                      \
	template <typename... Ts>                                                    \
	struct TaggedDtorBase</* NOLINT */                                           \
	                      NeedsDtor,                                             \
	                      Ts...> {                                               \
		using TagType = meta::conditional_t<sizeof...(Ts) < 256U, u8, usize>;      \
		template <usize I>                                                         \
		using Ith = ith<I, Ts...>;                                                 \
                                                                               \
		union {                                                                    \
			Empty _;                                                                 \
			Uwunion<Ts...> inner;                                                    \
		};                                                                         \
		TagType tag;                                                               \
                                                                               \
		VEG_INLINE VEG_CPP20(constexpr) TaggedDtorBase /* NOLINT */                \
				() VEG_NOEXCEPT : _{} {}                                               \
		template <usize I, typename Fn>                                            \
		VEG_INLINE constexpr TaggedDtorBase(                                       \
				EmplaceTag /*tag*/, UTag<I> tag, Fn fn, usize i = I)                   \
				VEG_NOEXCEPT_LIKE(VEG_FWD(fn)(UTag<I>{}))                              \
				: inner{tag, tag, VEG_FWD(fn)}, tag(TagType(i)) {}                     \
                                                                               \
		__VEG_PP_CAT(VEG_TAGGED_UWUNION_DTOR_, NeedsDtor)                          \
                                                                               \
		template <usize I>                                                         \
				constexpr auto get_ref() const                                         \
				& VEG_NOEXCEPT -> ith<I, Ts...> const& {                               \
			return (void)meta::unreachable_if(tag != I),                             \
			       UwunionGetImpl<I>::get(inner);                                    \
		}                                                                          \
	}

VEG_TAGGED_UWUNION_DEF(true);
VEG_TAGGED_UWUNION_DEF(false);

#undef VEG_TAGGED_UWUNION_DEF

template <bool NeedsMove, bool NeedsCopy, typename... Ts>
struct TaggedCopyMoveBase;

#define VEG_TAGGED_UWUNION_DEF(NeedsMove, NeedsCopy)                           \
	template <typename... Ts>                                                    \
	struct TaggedCopyMoveBase</* NOLINT */                                       \
	                          NeedsMove,                                         \
	                          NeedsCopy,                                         \
	                          Ts...> {                                           \
		template <usize I>                                                         \
		using Ith = ith<I, Ts...>;                                                 \
                                                                               \
		TaggedDtorBase<                                                            \
				!VEG_ALL_OF(VEG_CONCEPT(trivially_destructible<Ts>)),                  \
				Ts...>                                                                 \
				inner;                                                                 \
                                                                               \
		VEG_INLINE VEG_CPP20(constexpr) TaggedCopyMoveBase() = default;            \
		template <usize I, typename Fn>                                            \
		VEG_INLINE constexpr TaggedCopyMoveBase(                                   \
				EmplaceTag etag, UTag<I> itag, Fn fn, usize i = I)                     \
				VEG_NOEXCEPT_LIKE(VEG_FWD(fn)(UTag<I>{}))                              \
				: inner{etag, itag, VEG_FWD(fn), i} {}                                 \
                                                                               \
		auto operator=(TaggedCopyMoveBase&&) -> TaggedCopyMoveBase& = default;     \
		auto operator=(TaggedCopyMoveBase const&)                                  \
				-> TaggedCopyMoveBase& = default;                                      \
		__VEG_PP_CAT(VEG_TAGGED_UWUNION_MOVE_, NeedsMove)                          \
		(TaggedCopyMoveBase)                                                       \
				__VEG_PP_CAT(VEG_TAGGED_UWUNION_COPY_, NeedsCopy)(TaggedCopyMoveBase)  \
	}

VEG_TAGGED_UWUNION_DEF(true, true);
VEG_TAGGED_UWUNION_DEF(false, true);
VEG_TAGGED_UWUNION_DEF(true, false);
VEG_TAGGED_UWUNION_DEF(false, false);

template <typename From, typename To>
struct IdxConvertingFn {
	From&& value;
	VEG_INLINE constexpr auto operator()(void* /*unused*/)
			const&& VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_constructible<From, To>))
					-> To {
		return To(VEG_FWD(value));
	}
};

template <typename Base, bool NeedsCopy, typename... Ts>
struct NonTrivialCopyAssign {
	NonTrivialCopyAssign() = default;
	~NonTrivialCopyAssign() = default;
	NonTrivialCopyAssign(NonTrivialCopyAssign const& _rhs) = default;
	NonTrivialCopyAssign(NonTrivialCopyAssign&& _rhs) = default;
	auto operator=(NonTrivialCopyAssign&& _rhs)
			-> NonTrivialCopyAssign& = default;
	VEG_INLINE
	VEG_CPP14(constexpr)
	auto operator=(NonTrivialCopyAssign const& _rhs) /* NOLINT */ VEG_NOEXCEPT_IF(
			VEG_ALL_OF(VEG_CONCEPT(nothrow_copy_constructible<Ts>)) &&
			VEG_ALL_OF(VEG_CONCEPT(nothrow_copy_assignable<Ts>)))
			-> NonTrivialCopyAssign& {
		using Inner = decltype(static_cast<Base*>(this)->inner);
		using Fn = UwunionGetter<Inner const&>;
		auto& self = static_cast<Base&>(*this);
		auto& rhs = static_cast<Base const&>(_rhs);

		if (self.inner.tag == rhs.inner.tag) {
			internal::visit<
					void,
					VEG_ALL_OF(VEG_CONCEPT(nothrow_copy_assignable<Ts>)),
					sizeof...(Ts)>(self.inner.tag, AssignFn<Base, Fn>{self, {rhs.inner}});
		} else {
			Base local{rhs};
			self.inner.destroy();
			self.inner.tag = rhs.inner.tag;
			self.construct(UwunionGetter<Inner&&>{rhs.inner});
		}
		return *this;
	}
};
template <typename Base, bool NeedsMove, typename... Ts>
struct NonTrivialMoveAssign {
	NonTrivialMoveAssign() = default;
	~NonTrivialMoveAssign() = default;
	NonTrivialMoveAssign(NonTrivialMoveAssign const& _rhs) = default;
	NonTrivialMoveAssign(NonTrivialMoveAssign&& _rhs) = default;
	auto operator=(NonTrivialMoveAssign const& _rhs)
			-> NonTrivialMoveAssign& = default;
	VEG_INLINE
	VEG_CPP14(constexpr)
	auto operator=(NonTrivialMoveAssign&& _rhs) VEG_NOEXCEPT_IF(
			VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Ts>)) &&
			VEG_ALL_OF(VEG_CONCEPT(nothrow_move_assignable<Ts>)))
			-> NonTrivialMoveAssign& {
		using Inner = decltype(static_cast<Base*>(this)->inner);
		using Fn = UwunionGetter<Inner&&>;
		auto& self = static_cast<Base&>(*this);
		auto& rhs = static_cast<Base&>(_rhs);

		if (self.inner.tag == rhs.inner.tag) {
			internal::visit<
					void,
					VEG_ALL_OF(VEG_CONCEPT(nothrow_move_assignable<Ts>)),
					sizeof...(Ts)>(self.inner.tag, AssignFn<Base, Fn>{self, {rhs.inner}});
		} else {
			self.inner.destroy();
			self.inner.tag = rhs.inner.tag;
			self.construct(UwunionGetter<Inner&&>{rhs.inner});
		}
		return *this;
	}
};
template <typename Base, typename... Ts>
struct NonTrivialMoveAssign<Base, false, Ts...> {};
template <typename Base, typename... Ts>
struct NonTrivialCopyAssign<Base, false, Ts...> {};

template <bool NeedsMoveAssign, bool NeedsCopyAssign, typename T>
struct NoOpCopyMove;

template <typename T>
struct NoOpCopyMove<false, false, T> : T {
	using T::T;
};
template <typename T>
struct NoOpCopyMove<false, true, T> : T { /* NOLINT */
	using T::T;
	NoOpCopyMove(NoOpCopyMove const&) = default;
	NoOpCopyMove(NoOpCopyMove&&) = default;
	auto operator=(NoOpCopyMove&& /*unused*/) VEG_NOEXCEPT
			-> NoOpCopyMove& = default;
	auto operator= /* NOLINT */(NoOpCopyMove const& /*unused*/) VEG_NOEXCEPT
			-> NoOpCopyMove& {
		return *this;
	};
};
template <typename T>
struct NoOpCopyMove<true, false, T> : T { /* NOLINT */
	using T::T;
	NoOpCopyMove(NoOpCopyMove const&) = default;
	NoOpCopyMove(NoOpCopyMove&&) = default;
	auto operator=(NoOpCopyMove&& /*unused*/) VEG_NOEXCEPT -> NoOpCopyMove& {
		return *this;
	};
	auto operator=(NoOpCopyMove const& /*unused*/) VEG_NOEXCEPT
			-> NoOpCopyMove& = default;
};
template <typename T>
struct NoOpCopyMove<true, true, T> : T { /* NOLINT */
	using T::T;
	NoOpCopyMove(NoOpCopyMove const&) = default;
	NoOpCopyMove(NoOpCopyMove&&) = default;
	auto operator=(NoOpCopyMove&& /*unused*/) VEG_NOEXCEPT -> NoOpCopyMove& {
		return *this;
	};
	auto operator= /* NOLINT */(NoOpCopyMove const& /*unused*/) VEG_NOEXCEPT
			-> NoOpCopyMove& {
		return *this;
	};
};

#define VEG_NEEDS_MOVE                                                         \
	(VEG_ALL_OF(VEG_CONCEPT(move_constructible<Ts>)) &&                          \
	 !VEG_ALL_OF(VEG_CONCEPT(trivially_move_constructible<Ts>)))

#define VEG_NEEDS_COPY                                                         \
	(VEG_ALL_OF(VEG_CONCEPT(copy_constructible<Ts>)) &&                          \
	 !VEG_ALL_OF(VEG_CONCEPT(trivially_copy_constructible<Ts>)))

#define VEG_NEEDS_MOVE_ASSIGN                                                  \
	(!VEG_ALL_OF(VEG_CONCEPT(trivially_move_assignable<Ts>)) &&                  \
	 VEG_ALL_OF(VEG_CONCEPT(move_assignable<Ts>)) &&                             \
	 VEG_ALL_OF(VEG_CONCEPT(move_constructible<Ts>)))

#define VEG_NEEDS_COPY_ASSIGN                                                  \
	(!VEG_ALL_OF(VEG_CONCEPT(trivially_copy_assignable<Ts>)) &&                  \
	 VEG_ALL_OF(VEG_CONCEPT(copy_assignable<Ts>)) &&                             \
	 VEG_ALL_OF(VEG_CONCEPT(copy_constructible<Ts>)))

// TODO double storage uwunion
template <non_trivial_kind_e Kind, usize... Is, typename... Ts>
struct NonTrivialTaggedImpl<Kind, meta::index_sequence<Is...>, Ts...>
		: NoOpCopyMove<
					VEG_NEEDS_MOVE_ASSIGN,
					VEG_NEEDS_COPY_ASSIGN,
					TaggedCopyMoveBase<VEG_NEEDS_MOVE, VEG_NEEDS_COPY, Ts...>>,
			NonTrivialMoveAssign<
					NonTrivialTaggedImpl<Kind, meta::index_sequence<Is...>, Ts...>,
					VEG_NEEDS_MOVE_ASSIGN,
					Ts...>,
			NonTrivialCopyAssign<
					NonTrivialTaggedImpl<Kind, meta::index_sequence<Is...>, Ts...>,
					VEG_NEEDS_COPY_ASSIGN,
					Ts...> {
	template <usize I>
	using Ith = ith<I, Ts...>;

	using Self = NonTrivialTaggedImpl;
	using Base = NoOpCopyMove<
			VEG_NEEDS_MOVE_ASSIGN,
			VEG_NEEDS_COPY_ASSIGN,
			TaggedCopyMoveBase<VEG_NEEDS_MOVE, VEG_NEEDS_COPY, Ts...>>;

	// precondition: fn(UTag<inner.tag>{}) must be valid
	template <typename Fn>
	VEG_INLINE VEG_CPP20(constexpr) void construct(Fn fn)
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_IS_NOEXCEPT(VEG_FWD(fn)(UTag<Is>{})))) {
		internal::visit<
				void,
				VEG_ALL_OF(VEG_IS_NOEXCEPT(VEG_FWD(fn)(UTag<Is>{}))),
				sizeof...(Ts)>(
				this->inner.tag,
				CtorFn<Uwunion<Ts...>, Fn>{this->inner.inner, VEG_FWD(fn)});
	}

	using Base::Base;

	template <typename Fn>
	VEG_INLINE VEG_CPP14(constexpr) void assign(Fn fn) VEG_NOEXCEPT_IF(VEG_ALL_OF(
			VEG_IS_NOEXCEPT(*static_cast<Ts*>(nullptr) = VEG_FWD(fn)(UTag<Is>{})))) {
		internal::visit<
				void,
				VEG_ALL_OF(VEG_IS_NOEXCEPT(
						*static_cast<Ts*>(nullptr) = VEG_FWD(fn)(UTag<Is>{}))),
				sizeof...(Ts)>(this->inner.tag, AssignFn<Self, Fn>{*this, VEG_FWD(fn)});
	}

	template <usize I, typename Fn>
	VEG_INLINE VEG_CPP20(constexpr) void uninit_emplace(Fn fn)
			VEG_NOEXCEPT_LIKE(VEG_FWD(fn)(UTag<I>{})) {
		mem::construct_at(
				mem::addressof(this->inner.inner), UTag<I>{}, UTag<I>{}, VEG_FWD(fn));
	}

	template <usize I, typename Fn>
	VEG_INLINE VEG_CPP20(constexpr) void _emplace(Fn fn)
			VEG_NOEXCEPT_LIKE(VEG_FWD(fn)(UTag<I>{})) {
		using T = Ith<I>;
		T local{VEG_FWD(fn)(UTag<I>{})};
		{
			// this block never throws
			this->inner.destroy();
			uninit_emplace<I>(IdxConvertingFn<T&&, T>{VEG_FWD(local)});
		}
		this->inner.tag = I;
	}

	template <usize I>
			constexpr auto get_ref() const & VEG_NOEXCEPT -> ith<I, Ts...> const& {
		return (void)meta::unreachable_if(this->inner.tag != I),
		       UwunionGetImpl<I>::get(this->inner.inner);
	}
	VEG_INLINE constexpr auto index() const -> usize { return this->inner.tag; }
};

template <typename... Ts>
struct NonTrivialTaggedUwunion {
	using Type = NonTrivialTaggedImpl<
			!(VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Ts>))) //
					? needs_double_storage
					: !(VEG_ALL_OF(VEG_CONCEPT(trivially_destructible<Ts>))) //
								? needs_dtor
								: needs_copy_move,
			meta::make_index_sequence<sizeof...(Ts)>,
			Ts...>;
};

template <typename... Ts>
struct TrivialTaggedUwunion {
	using Type =
			TrivialTaggedImpl<meta::make_index_sequence<sizeof...(Ts)>, Ts...>;
};

template <typename... Ts>
using TaggedUwunionImpl = typename meta::conditional_t<
		VEG_ALL_OF(VEG_CONCEPT(trivially_copyable<Storage<Ts>>)),
		TrivialTaggedUwunion<Ts...>,
		NonTrivialTaggedUwunion<Ts...>>::Type;
;

template <typename Fn>
struct TaggedFn {
	Fn&& fn;
	VEG_INLINE constexpr auto
	operator()(void* /*unused*/) const&& VEG_DEDUCE_RET(VEG_FWD(fn)());
};
} // namespace _uwunion
} // namespace internal

template <typename... Ts>
struct Uwunion : internal::_uwunion::TaggedUwunionImpl<Ts...> {
private:
	template <typename... Us>
	friend struct veg::Uwunion;
	template <typename, typename>
	friend struct internal::_uwunion::EmplaceWrapper;

	using Base = internal::_uwunion::TaggedUwunionImpl<Ts...>;
	using Base::Base;

public:
	VEG_EXPLICIT_COPY(Uwunion);

	VEG_TEMPLATE(
			(i64 I),
			requires(VEG_CONCEPT(move_constructible<ith<usize{I}, Ts...>>)),
			VEG_INLINE constexpr Uwunion,
			(/*tag*/, Fix<I>),
			(arg, ith<usize{I}, Ts...>))
	VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_move_constructible<ith<usize{I}, Ts...>>))
			: Base{
						internal::_uwunion::EmplaceTag{},
						internal::UTag<usize{I}>{},
						internal::_uwunion::
								IdxConvertingFn<ith<usize{I}, Ts...>&&, ith<usize{I}, Ts...>>{
										VEG_FWD(arg)},
				} {}

	VEG_TEMPLATE(
			(i64 I, typename Fn),
			requires(VEG_CONCEPT(invocable_r<Fn, ith<I, Ts...>>)),
			VEG_INLINE constexpr Uwunion,
			(/*inplace*/, InPlace),
			(/*tag*/, Fix<I>),
			(fn, Fn))
	VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_invocable<Fn>))
			: Base{
						internal::_uwunion::EmplaceTag{},
						internal::UTag<usize{I}>{},
						internal::_uwunion::TaggedFn<Fn&&>{VEG_FWD(fn)},
						usize{I},
				} {}

	VEG_TEMPLATE(
			(i64 I, typename Fn),
			requires(
					usize{I} < sizeof...(Ts) &&
					VEG_CONCEPT(invocable_r<Fn, ith<usize{I}, Ts...>>)),
			VEG_INLINE VEG_CPP20(constexpr) auto emplace,
			(/*tag*/, Fix<I>),
			(fn, Fn))
	VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_invocable<Fn>))->ith<usize{I}, Ts...>& {
		this->template _emplace<usize{I}>(
				internal::_uwunion::TaggedFn<Fn&&>{VEG_FWD(fn)});
		return const_cast<ith<usize{I}, Ts...>&>(this->template get_ref<I>());
	}

	VEG_TEMPLATE(
			(i64 I),
			requires(usize{I} < sizeof...(Ts)),
			VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto unwrap,
			(/*tag*/, Fix<I>)) &&
			VEG_NOEXCEPT_IF(
					VEG_CONCEPT(nothrow_move_constructible<ith<usize{I}, Ts...>>))
					-> ith<usize{I}, Ts...> {
		VEG_INTERNAL_ASSERT_PRECONDITION(I == index());
		return static_cast<Uwunion&&>(*this).unwrap_unchecked(Fix<I>{}, unsafe);
	}

	VEG_TEMPLATE(
			(i64 I),
			requires(usize{I} < sizeof...(Ts)),
			VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto unwrap_unchecked,
			(/*tag*/, Fix<I>),
			(/*tag*/, Unsafe)) &&
			VEG_NOEXCEPT_IF(
					VEG_CONCEPT(nothrow_move_constructible<ith<usize{I}, Ts...>>))
					-> ith<usize{I}, Ts...> {
		meta::unreachable_if(usize{I} != usize(index()));
		return const_cast<ith<usize{I}, Ts...>&&>(this->template get_ref<I>());
	}

	using Base::index;

	void as_ref() const&& = delete;

	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto as_ref() & //
			VEG_NOEXCEPT -> Uwunion<Ts&...> {
		return internal::visit14<Uwunion<Ts&...>, true, sizeof...(Ts)>(
				usize(index()),
				internal::_uwunion::EmplaceWrapper<
						Uwunion<Ts&...>,
						internal::_uwunion::UwunionGetter<Base&>>{{*this}, usize(index())});
	}
	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto as_ref() const & //
			VEG_NOEXCEPT -> Uwunion<Ts const&...> {
		return internal::visit14<Uwunion<Ts const&...>, true, sizeof...(Ts)>(
				usize(index()),
				internal::_uwunion::EmplaceWrapper<
						Uwunion<Ts const&...>,
						internal::_uwunion::UwunionGetter<Base const&>>{
						{*this}, usize(index())});
	}
	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto as_ref() && //
			VEG_NOEXCEPT -> Uwunion<Ts&&...> {
		return internal::visit14<Uwunion<Ts&&...>, true, sizeof...(Ts)>(
				usize(index()),
				internal::_uwunion::EmplaceWrapper<
						Uwunion<Ts&&...>,
						internal::_uwunion::UwunionGetter<Base&&>>{
						{*this}, usize(index())});
	}
};
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_UWUNION_HPP_OHC4GK5JS */
