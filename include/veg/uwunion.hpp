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
union RawUwunionImpl;

template <typename... Ts>
using RawUwunion = RawUwunionImpl<false, Ts...>;
template <usize I>
struct UwunionGetImpl;

template <>
union RawUwunionImpl<false> {
	Empty _{};
};

#define VEG_TYPE_DECL(_, I) , typename __VEG_PP_CAT(T, I)
#define VEG_TYPE_PUT(_, I) , __VEG_PP_CAT(T, I)
#define VEG_UWUNION_HEAD(_, I)                                                 \
	Storage<__VEG_PP_CAT(T, I)> __VEG_PP_CAT(head, I);
#define VEG_UWUNION_HEAD_CTOR(_, I)                                            \
	template <usize J, typename Fn>                                              \
	VEG_INLINE constexpr RawUwunionImpl(UTag<I> /*unused*/, UTag<J>, Fn fn)      \
			VEG_NOEXCEPT_LIKE(VEG_FWD(fn)(UTag<J>{}))                                \
			: __VEG_PP_CAT(head, I){                                                 \
						static_cast<decltype(__VEG_PP_CAT(head, I).inner)>(                \
								VEG_FWD(fn)(UTag<J>{}))} {}

#define VEG_UWUNION_SPEC(Tuple)                                                \
	template <bool _ __VEG_PP_TUPLE_FOR_EACH(VEG_TYPE_DECL, _, Tuple)>           \
	union RawUwunionImpl<_ __VEG_PP_TUPLE_FOR_EACH(VEG_TYPE_PUT, _, Tuple)> {    \
		__VEG_PP_TUPLE_FOR_EACH(VEG_UWUNION_HEAD, _, Tuple)                        \
		__VEG_PP_TUPLE_FOR_EACH(VEG_UWUNION_HEAD_CTOR, _, Tuple)                   \
	};                                                                           \
	template <>                                                                  \
	struct UwunionGetImpl<__VEG_PP_TUPLE_SIZE(Tuple)> {                          \
		template <typename... Ts>                                                  \
		VEG_INLINE static constexpr auto                                           \
		get(RawUwunion<Ts...> const& u) VEG_NOEXCEPT                               \
				-> Storage<ith<__VEG_PP_TUPLE_SIZE(Tuple), Ts...>> const& {            \
			return u.__VEG_PP_CAT(head, __VEG_PP_TUPLE_SIZE(Tuple));                 \
		}                                                                          \
	};

#define VEG_UWUNION_TPL(Tuple)

template <>
struct UwunionGetImpl<0> {
	template <typename... Ts>
	VEG_INLINE static constexpr auto get(RawUwunion<Ts...> const& u) VEG_NOEXCEPT
			-> Storage<ith<0, Ts...>> const& {
		return u.head0;
	}
};

VEG_UWUNION_SPEC(__VEG_PP_MAKE_TUPLE(1));
VEG_UWUNION_SPEC(__VEG_PP_MAKE_TUPLE(2));
VEG_UWUNION_SPEC(__VEG_PP_MAKE_TUPLE(3));
VEG_UWUNION_SPEC(__VEG_PP_MAKE_TUPLE(4));
VEG_UWUNION_SPEC(__VEG_PP_MAKE_TUPLE(5));
VEG_UWUNION_SPEC(__VEG_PP_MAKE_TUPLE(6));
VEG_UWUNION_SPEC(__VEG_PP_MAKE_TUPLE(7));
VEG_UWUNION_SPEC(__VEG_PP_MAKE_TUPLE(8));
VEG_UWUNION_SPEC(__VEG_PP_MAKE_TUPLE(9));
VEG_UWUNION_SPEC(__VEG_PP_MAKE_TUPLE(10));
VEG_UWUNION_SPEC(__VEG_PP_MAKE_TUPLE(11));
VEG_UWUNION_SPEC(__VEG_PP_MAKE_TUPLE(12));
VEG_UWUNION_SPEC(__VEG_PP_MAKE_TUPLE(13));
VEG_UWUNION_SPEC(__VEG_PP_MAKE_TUPLE(14));
VEG_UWUNION_SPEC(__VEG_PP_MAKE_TUPLE(15));
#define VEG_TUPLE __VEG_PP_MAKE_TUPLE(16)

template <
		bool _ __VEG_PP_TUPLE_FOR_EACH(VEG_TYPE_DECL, _, VEG_TUPLE),
		typename... Ts>
union RawUwunionImpl<
		_ __VEG_PP_TUPLE_FOR_EACH(VEG_TYPE_PUT, _, VEG_TUPLE),
		Ts...> {
	RawUwunion<Ts...> tail;
	__VEG_PP_TUPLE_FOR_EACH(VEG_UWUNION_HEAD, _, VEG_TUPLE)
	__VEG_PP_TUPLE_FOR_EACH(VEG_UWUNION_HEAD_CTOR, _, VEG_TUPLE)
	template <usize I, usize J, typename Fn>
	VEG_INLINE constexpr RawUwunionImpl(UTag<I> /*unused*/, UTag<J> itag, Fn fn)
			VEG_NOEXCEPT_LIKE(VEG_FWD(fn)(UTag<J>{}))
			: tail{UTag<I - __VEG_PP_TUPLE_SIZE(VEG_TUPLE)>{}, itag, VEG_FWD(fn)} {}
};
template <usize I>
struct UwunionGetImpl {
	template <typename... Ts>
	VEG_INLINE static constexpr auto get(RawUwunion<Ts...> const& u) VEG_NOEXCEPT
			-> Storage<ith<I, Ts...>> const& {
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
struct NonTrivialUwunionImpl;

template <typename U, typename... Ts>
struct UwunionGetter;

template <typename U, typename... Ts>
struct UwunionGetter<U const&, Ts...> {
	U const& self;
	template <usize I>
	VEG_INLINE constexpr auto operator()(UTag<I> /*unused*/) const noexcept
			-> ith<I, Ts...> const& {
		return UwunionGetImpl<I>::get(self).inner;
	}
};
template <typename U, typename... Ts>
struct UwunionGetter<U&, Ts...> {
	U& self;
	template <usize I>
	VEG_INLINE constexpr auto operator()(UTag<I> /*unused*/) const noexcept
			-> ith<I, Ts...>& {
		return const_cast<ith<I, Ts...>&>(UwunionGetImpl<I>::get(self).inner);
	}
};
template <typename U, typename... Ts>
struct UwunionGetter<U&&, Ts...> {
	U& self;
	template <usize I>
	VEG_INLINE constexpr auto operator()(UTag<I> /*unused*/) const noexcept
			-> ith<I, Ts...>&& {
		return const_cast<ith<I, Ts...>&&>(UwunionGetImpl<I>::get(self).inner);
	}
};

struct EmplaceTag {};

template <typename Target, typename Fn>
struct EmplaceWrapper {
	Fn const fn;
	usize i;
	template <usize I>
	VEG_INLINE constexpr auto
	operator()(UTag<I> itag) const&& VEG_NOEXCEPT_LIKE(VEG_FWD(fn)(UTag<I>{}))
			-> Target {
		return {EmplaceTag{}, itag, VEG_FWD(fn), i};
	}
};

template <typename U, typename Fn>
struct CtorFn {
	U& self;
	Fn fn;

	template <usize I>
			VEG_INLINE VEG_CPP20(constexpr) void operator()(UTag<I> itag) &&
			VEG_NOEXCEPT_LIKE(VEG_FWD(fn)(itag)) {
		mem::construct_at(mem::addressof(self), itag, itag, VEG_FWD(fn));
	};
};

template <typename U, typename Fn, typename... Ts>
struct AssignFn {
	U& self;
	Fn fn;

	template <usize I>
			VEG_INLINE VEG_CPP14(constexpr) void operator()(UTag<I> itag) &&
			VEG_NOEXCEPT_LIKE(VEG_FWD(fn)(UTag<I>{})) {
		const_cast<ith<I, Ts...>&>(UwunionGetImpl<I>::get(self).inner) =
				VEG_FWD(fn)(itag);
	};
};

template <bool NoExcept, typename U, typename... Ts>
struct DropFn {
	U& self;

	template <usize I>
	VEG_INLINE VEG_CPP20(constexpr) void
	operator()(UTag<I> /*itag*/) const noexcept {
		mem::destroy_at(mem::addressof(
				const_cast<Storage<ith<I, Ts...>>&>(UwunionGetImpl<I>::get(self))));
	};
};

template <typename... Ts>
struct TrivialUwunionImpl {
	template <usize I>
	using Ith = ith<I, Ts...>;
	using TagType = meta::conditional_t<sizeof...(Ts) < 256U, u8, usize>;

	static_assert(VEG_ALL_OF(VEG_CONCEPT(trivially_copyable<Storage<Ts>>)), ".");
	union {
		Empty _;
		RawUwunion<Ts...> inner;
	};
	TagType tag;

	template <usize I, typename Fn>
	VEG_INLINE constexpr TrivialUwunionImpl(
			EmplaceTag /*etag*/, UTag<I> itag, Fn fn, usize i = I)
			VEG_NOEXCEPT_LIKE(VEG_FWD(fn)(UTag<I>{}))
			: inner{itag, itag, VEG_FWD(fn)}, tag(TagType(i)) {}

	template <usize I, typename Fn>
	VEG_INLINE VEG_CPP20(constexpr) void _emplace(Fn fn)
			VEG_NOEXCEPT_LIKE(VEG_FWD(fn)(UTag<I>{})) {
		mem::construct_at(mem::addressof(inner), UTag<I>{}, UTag<I>{}, VEG_FWD(fn));
		tag = I;
	}

	template <usize I>
			VEG_INLINE constexpr auto get_ref() const
			& VEG_NOEXCEPT -> Ith<I> const& {
		return (void)meta::unreachable_if(tag != I),
		       UwunionGetImpl<I>::get(inner).inner;
	}

	VEG_NODISCARD VEG_INLINE constexpr auto index() const -> usize { return tag; }
	VEG_INLINE constexpr auto get_union_ref() const VEG_NOEXCEPT
			-> RawUwunion<Ts...> const& {
		return this->inner;
	}
};

template <typename Base, usize N, bool NoExcept, typename Fn>
VEG_INLINE constexpr auto make(Fn fn, usize tag) VEG_NOEXCEPT_IF(NoExcept)
		-> Base {
	return internal::visit<Base, NoExcept, N>(
			tag, EmplaceWrapper<Base, Fn>{VEG_FWD(fn), tag});
}

template <bool NeedsDtor, typename... Ts>
struct NonTrivialUwunionDtor;

#define VEG_TAGGED_UWUNION_DTOR_true                                           \
	VEG_INLINE VEG_CPP14(constexpr) void destroy() VEG_NOEXCEPT {                \
		internal::visit<void, true, sizeof...(Ts)>(                                \
				tag, DropFn<true, decltype(inner), Ts...>{inner});                     \
	}                                                                            \
	VEG_INLINE VEG_CPP20(constexpr) ~NonTrivialUwunionDtor() { destroy(); }

#define VEG_TAGGED_UWUNION_DTOR_false                                          \
	VEG_INLINE VEG_CPP14(constexpr) void destroy() VEG_NOEXCEPT {}

#define VEG_TAGGED_UWUNION_MOVE_false(Class)                                   \
	Class(Class&&) = default; /* NOLINT */
#define VEG_TAGGED_UWUNION_COPY_false(Class) Class(Class const&) = default;

#if __cplusplus >= 201703L

#define VEG_TAGGED_UWUNION_MOVE_true(Class)                                    \
	VEG_INLINE                                                                   \
	constexpr Class(                                                             \
			__VEG_PP_REMOVE_PAREN(Class) &&                                          \
			rhs) /* silence warning about unparenthesized macro argument */          \
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Ts>))) \
			: inner{_uwunion::make<                                                  \
						decltype(inner),                                                   \
						sizeof...(Ts),                                                     \
						VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Ts>))>(          \
						UwunionGetter<decltype(inner.inner)&&, Ts...>{rhs.inner.inner},    \
						rhs.inner.tag)} {}

#define VEG_TAGGED_UWUNION_COPY_true(Class)                                    \
	VEG_INLINE                                                                   \
	constexpr Class(Class const& rhs)                                            \
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_copy_constructible<Ts>))) \
			: inner{_uwunion::make<                                                  \
						decltype(inner),                                                   \
						sizeof...(Ts),                                                     \
						VEG_ALL_OF(VEG_CONCEPT(nothrow_copy_constructible<Ts>))>(          \
						UwunionGetter<decltype(inner.inner) const&, Ts...>{                \
								rhs.inner.inner},                                              \
						rhs.inner.tag)} {}

#else

#define VEG_TAGGED_UWUNION_MOVE_true(Class)                                    \
	VEG_INLINE                                                                   \
	Class(Class&& rhs) /* NOLINT */                                              \
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Ts>))) \
			: inner{} {                                                              \
		using Fn = UwunionGetter<decltype(inner.inner)&&, Ts...>;                  \
		inner.tag = rhs.inner.tag;                                                 \
		internal::visit<                                                           \
				void,                                                                  \
				VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Ts>)),               \
				sizeof...(Ts)>(                                                        \
				this->inner.tag,                                                       \
				CtorFn<decltype(inner.inner), Fn>{inner.inner, Fn{rhs.inner.inner}});  \
	}

#define VEG_TAGGED_UWUNION_COPY_true(Class)                                    \
	VEG_INLINE                                                                   \
	Class(Class const& rhs)                                                      \
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_copy_constructible<Ts>))) \
			: inner{} {                                                              \
		using Fn = UwunionGetter<decltype(inner.inner) const&, Ts...>;             \
		inner.tag = rhs.inner.tag;                                                 \
		internal::visit<                                                           \
				void,                                                                  \
				VEG_ALL_OF(VEG_CONCEPT(nothrow_copy_constructible<Ts>)),               \
				sizeof...(Ts)>(                                                        \
				this->inner.tag,                                                       \
				CtorFn<decltype(inner.inner), Fn>{inner.inner, Fn{rhs.inner.inner}});  \
	}

#endif

#define VEG_TAGGED_UWUNION_DEF(NeedsDtor)                                      \
	template <typename... Ts>                                                    \
	struct NonTrivialUwunionDtor</* NOLINT */                                    \
	                             NeedsDtor,                                      \
	                             Ts...> {                                        \
		using TagType = meta::conditional_t<sizeof...(Ts) < 256U, u8, usize>;      \
		template <usize I>                                                         \
		using Ith = ith<I, Ts...>;                                                 \
                                                                               \
		union {                                                                    \
			Empty _;                                                                 \
			RawUwunion<Ts...> inner;                                                 \
		};                                                                         \
		TagType tag;                                                               \
                                                                               \
		VEG_INLINE VEG_CPP20(constexpr) NonTrivialUwunionDtor /* NOLINT */         \
				() VEG_NOEXCEPT : _{} {}                                               \
		template <usize I, typename Fn>                                            \
		VEG_INLINE constexpr NonTrivialUwunionDtor(                                \
				EmplaceTag /*etag*/, UTag<I> itag, Fn fn, usize i = I)                 \
				VEG_NOEXCEPT_LIKE(VEG_FWD(fn)(UTag<I>{}))                              \
				: inner{itag, itag, VEG_FWD(fn)}, tag(TagType(i)) {}                   \
                                                                               \
		__VEG_PP_CAT(VEG_TAGGED_UWUNION_DTOR_, NeedsDtor)                          \
                                                                               \
		template <usize I>                                                         \
				VEG_INLINE constexpr auto get_ref() const                              \
				& VEG_NOEXCEPT -> ith<I, Ts...> const& {                               \
			return (void)meta::unreachable_if(tag != I),                             \
			       UwunionGetImpl<I>::get(inner).inner;                              \
		}                                                                          \
	}

VEG_TAGGED_UWUNION_DEF(true);
VEG_TAGGED_UWUNION_DEF(false);

#undef VEG_TAGGED_UWUNION_DEF

template <bool NeedsMove, bool NeedsCopy, typename... Ts>
struct NonTrivialUwunionCopyMove;

#define VEG_TAGGED_UWUNION_DEF(NeedsMove, NeedsCopy)                           \
	template <typename... Ts>                                                    \
	struct NonTrivialUwunionCopyMove</* NOLINT */                                \
	                                 NeedsMove,                                  \
	                                 NeedsCopy,                                  \
	                                 Ts...> {                                    \
		template <usize I>                                                         \
		using Ith = ith<I, Ts...>;                                                 \
                                                                               \
		NonTrivialUwunionDtor<                                                     \
				!VEG_ALL_OF(VEG_CONCEPT(trivially_destructible<Ts>)),                  \
				Ts...>                                                                 \
				inner;                                                                 \
                                                                               \
		VEG_INLINE VEG_CPP20(constexpr) NonTrivialUwunionCopyMove() = default;     \
		template <usize I, typename Fn>                                            \
		VEG_INLINE constexpr NonTrivialUwunionCopyMove(                            \
				EmplaceTag etag, UTag<I> itag, Fn fn, usize i = I)                     \
				VEG_NOEXCEPT_LIKE(VEG_FWD(fn)(UTag<I>{}))                              \
				: inner{etag, itag, VEG_FWD(fn), i} {}                                 \
                                                                               \
		auto operator=(NonTrivialUwunionCopyMove&&)                                \
				-> NonTrivialUwunionCopyMove& = default;                               \
		auto operator=(NonTrivialUwunionCopyMove const&)                           \
				-> NonTrivialUwunionCopyMove& = default;                               \
		__VEG_PP_CAT(VEG_TAGGED_UWUNION_MOVE_, NeedsMove)                          \
		(NonTrivialUwunionCopyMove) __VEG_PP_CAT(                                  \
				VEG_TAGGED_UWUNION_COPY_, NeedsCopy)(NonTrivialUwunionCopyMove)        \
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
			VEG_ALL_OF(VEG_CONCEPT(nothrow_copy_assignable<Storage<Ts>>)))
			-> NonTrivialCopyAssign& {
		using Inner = decltype(static_cast<Base*>(this)->inner.inner);
		using Fn = UwunionGetter<Inner const&, Ts...>;
		auto& self = static_cast<Base&>(*this);
		auto& rhs = static_cast<Base const&>(_rhs);

		if (self.inner.tag == rhs.inner.tag) {
			internal::visit<
					void,
					VEG_ALL_OF(VEG_CONCEPT(nothrow_copy_assignable<Storage<Ts>>)),
					sizeof...(Ts)>(
					self.inner.tag,
					AssignFn<Base, Fn, Ts...>{self.inner.inner, {rhs.inner.inner}});
		} else {
			Base local{rhs};
			self.inner.destroy();
			self.inner.tag = rhs.inner.tag;
			self.construct(UwunionGetter<Inner&&, Ts...>{rhs.inner.inner});
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
			VEG_ALL_OF(VEG_CONCEPT(nothrow_move_assignable<Storage<Ts>>)))
			-> NonTrivialMoveAssign& {
		using Inner = decltype(static_cast<Base*>(this)->inner.inner);
		using Fn = UwunionGetter<Inner&&, Ts...>;
		auto& self = static_cast<Base&>(*this);
		auto& rhs = static_cast<Base&>(_rhs);

		if (self.inner.tag == rhs.inner.tag) {
			internal::visit<
					void,
					VEG_ALL_OF(VEG_CONCEPT(nothrow_move_assignable<Storage<Ts>>)),
					sizeof...(Ts)>(
					self.inner.tag,
					AssignFn<Base, Fn, Ts...>{self.inner.inner, {rhs.inner.inner}});
		} else {
			self.inner.destroy();
			self.inner.tag = rhs.inner.tag;
			self.construct(UwunionGetter<Inner&&, Ts...>{rhs.inner.inner});
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
	(!VEG_ALL_OF(VEG_CONCEPT(trivially_move_assignable<Storage<Ts>>)) &&         \
	 VEG_ALL_OF(                                                                 \
			 (VEG_CONCEPT(move_constructible<Ts>) &&                                 \
	      VEG_CONCEPT(move_assignable<Storage<Ts>>))))

#define VEG_NEEDS_COPY_ASSIGN                                                  \
	(!VEG_ALL_OF(VEG_CONCEPT(trivially_copy_assignable<Storage<Ts>>)) &&         \
	 VEG_ALL_OF(                                                                 \
			 (VEG_CONCEPT(copy_constructible<Ts>) &&                                 \
	      VEG_CONCEPT(copy_assignable<Storage<Ts>>))))

// TODO double storage uwunion
template <non_trivial_kind_e Kind, usize... Is, typename... Ts>
struct NonTrivialUwunionImpl<Kind, meta::index_sequence<Is...>, Ts...>
		: NoOpCopyMove<
					VEG_NEEDS_MOVE_ASSIGN,
					VEG_NEEDS_COPY_ASSIGN,
					NonTrivialUwunionCopyMove<VEG_NEEDS_MOVE, VEG_NEEDS_COPY, Ts...>>,
			NonTrivialMoveAssign<
					NonTrivialUwunionImpl<Kind, meta::index_sequence<Is...>, Ts...>,
					VEG_NEEDS_MOVE_ASSIGN,
					Ts...>,
			NonTrivialCopyAssign<
					NonTrivialUwunionImpl<Kind, meta::index_sequence<Is...>, Ts...>,
					VEG_NEEDS_COPY_ASSIGN,
					Ts...> {
	template <usize I>
	using Ith = ith<I, Ts...>;

	VEG_INLINE constexpr auto get_union_ref() const VEG_NOEXCEPT
			-> RawUwunion<Ts...> const& {
		return this->inner.inner;
	}

	using Self = NonTrivialUwunionImpl;
	using Base = NoOpCopyMove<
			VEG_NEEDS_MOVE_ASSIGN,
			VEG_NEEDS_COPY_ASSIGN,
			NonTrivialUwunionCopyMove<VEG_NEEDS_MOVE, VEG_NEEDS_COPY, Ts...>>;

	// precondition: fn(UTag<inner.tag>{}) must be valid
	template <typename Fn>
	VEG_INLINE VEG_CPP20(constexpr) void construct(Fn fn)
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_IS_NOEXCEPT(VEG_FWD(fn)(UTag<Is>{})))) {
		internal::visit<
				void,
				VEG_ALL_OF(VEG_IS_NOEXCEPT(VEG_FWD(fn)(UTag<Is>{}))),
				sizeof...(Ts)>(
				this->inner.tag,
				CtorFn<RawUwunion<Ts...>, Fn>{this->inner.inner, VEG_FWD(fn)});
	}

	using Base::Base;

	template <usize I, typename Fn>
	VEG_INLINE VEG_CPP20(constexpr) void _emplace(Fn fn)
			VEG_NOEXCEPT_LIKE(VEG_FWD(fn)(UTag<I>{})) {
		using T = Ith<I>;
		T local{VEG_FWD(fn)(UTag<I>{})};
		{
			// this block never throws
			this->inner.destroy();
			mem::construct_at(
					mem::addressof(this->inner.inner),
					UTag<I>{},
					UTag<I>{},
					IdxConvertingFn<T&&, T>{VEG_FWD(local)});
		}
		this->inner.tag = I;
	}

	template <usize I>
			VEG_INLINE constexpr auto get_ref() const
			& VEG_NOEXCEPT -> ith<I, Ts...> const& {
		return (void)meta::unreachable_if(this->inner.tag != I),
		       UwunionGetImpl<I>::get(this->inner.inner).inner;
	}
	VEG_NODISCARD VEG_INLINE constexpr auto index() const -> usize {
		return this->inner.tag;
	}
};

template <typename... Ts>
struct DoubleStorageDtor { /* NOLINT */
	using TagType = meta::conditional_t<sizeof...(Ts) < 128U, u8, usize>;
	template <usize I>
	using Ith = ith<I, Ts...>;

	union {
		Empty _0;
		RawUwunion<Ts...> inner0;
	};
	union {
		Empty _1;
		RawUwunion<Ts...> inner1;
	};
	TagType tag_with_bit;

	VEG_INLINE VEG_CPP20(constexpr) DoubleStorageDtor() VEG_NOEXCEPT : _0{} {}
	template <usize I, typename Fn>
	VEG_INLINE constexpr DoubleStorageDtor(
			EmplaceTag /*etag*/, UTag<I> itag, Fn fn, usize i = I)
			VEG_NOEXCEPT_LIKE(VEG_FWD(fn)(UTag<I>{}))
			: inner0{itag, itag, VEG_FWD(fn)}, tag_with_bit(TagType(2U * i)) {}

	VEG_CPP20(constexpr)
	void destroy()
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_destructible<Ts>))) {
		internal::visit14<
				void,
				VEG_ALL_OF(VEG_CONCEPT(nothrow_destructible<Ts>)),
				sizeof...(Ts)>(
				tag_with_bit / 2U,
				DropFn<
						VEG_ALL_OF(VEG_CONCEPT(nothrow_destructible<Ts>)),
						decltype(inner0),
						Ts...>{tag_with_bit % 2U == 0 ? inner0 : inner1});
	}
	VEG_CPP20(constexpr)
	void destroy_inactive(usize tag)
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_destructible<Ts>))) {
		internal::visit14<
				void,
				VEG_ALL_OF(VEG_CONCEPT(nothrow_destructible<Ts>)),
				sizeof...(Ts)>(
				tag,
				DropFn<
						VEG_ALL_OF(VEG_CONCEPT(nothrow_destructible<Ts>)),
						decltype(inner0),
						Ts...>{tag_with_bit % 2U == 0 ? inner1 : inner0});
	}

	VEG_INLINE VEG_CPP20(constexpr) ~DoubleStorageDtor()
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_destructible<Ts>))) {
		this->destroy();
	}
};

template <typename... Ts>
struct DoubleStorageCopyMove { /* NOLINT */
	using TagType = meta::conditional_t<sizeof...(Ts) < 128U, u8, usize>;
	template <usize I>
	using Ith = ith<I, Ts...>;

	DoubleStorageDtor<Ts...> inner;

	DoubleStorageCopyMove() = default;
	template <usize I, typename Fn>
	VEG_INLINE constexpr DoubleStorageCopyMove(
			EmplaceTag etag, UTag<I> itag, Fn fn, usize i = I)
			VEG_NOEXCEPT_LIKE(VEG_FWD(fn)(UTag<I>{}))
			: inner{etag, itag, VEG_FWD(fn), i} {}

#if __cplusplus >= 201703L
	VEG_INLINE constexpr DoubleStorageCopyMove(DoubleStorageCopyMove&& rhs)
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Ts>)))
			: inner{_uwunion::make<
						decltype(inner),
						sizeof...(Ts),
						VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Ts>))>(
						UwunionGetter<decltype(inner.inner0)&&, Ts...>{
								(rhs.inner.tag_with_bit % 2U == 0) ? rhs.inner.inner0
																									 : rhs.inner.inner1},
						rhs.inner.tag_with_bit / 2U)} {}

	VEG_INLINE constexpr DoubleStorageCopyMove(DoubleStorageCopyMove const& rhs)
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_copy_constructible<Ts>)))
			: inner{_uwunion::make<
						decltype(inner),
						sizeof...(Ts),
						VEG_ALL_OF(VEG_CONCEPT(nothrow_copy_constructible<Ts>))>(
						UwunionGetter<decltype(inner) const&, Ts...>{
								(rhs.inner.tag_with_bit % 2U == 0) ? rhs.inner.inner0
																									 : rhs.inner.inner1},
						rhs.inner.tag_with_bit / 2U)} {}
#else
	VEG_INLINE
	VEG_CPP20(constexpr)
	DoubleStorageCopyMove(DoubleStorageCopyMove&& rhs)
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Ts>)))
			: inner{} {
		using Fn = UwunionGetter<decltype(inner.inner0)&&, Ts...>;
		inner.tag_with_bit = rhs.inner.tag_with_bit / 2U * 2U;
		internal::visit<
				void,
				VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Ts>)),
				sizeof...(Ts)>(
				inner.tag_with_bit / 2U,
				CtorFn<decltype(inner.inner0), Fn>{
						inner.inner0,
						Fn{(rhs.inner.tag_with_bit % 2U == 0) ? rhs.inner.inner0
		                                              : rhs.inner.inner1}});
	}

	VEG_INLINE
	VEG_CPP20(constexpr)
	DoubleStorageCopyMove(DoubleStorageCopyMove const& rhs)
			VEG_NOEXCEPT_IF(VEG_ALL_OF(VEG_CONCEPT(nothrow_copy_constructible<Ts>)))
			: inner{} {
		using Fn = UwunionGetter<decltype(inner.inner0) const&, Ts...>;
		inner.tag_with_bit = rhs.inner.tag_with_bit / 2U * 2U;
		internal::visit<
				void,
				VEG_ALL_OF(VEG_CONCEPT(nothrow_copy_constructible<Ts>)),
				sizeof...(Ts)>(
				inner.tag_with_bit / 2U,
				CtorFn<decltype(inner.inner0), Fn>{
						inner.inner0,
						Fn{(rhs.inner.tag_with_bit % 2U == 0) ? rhs.inner.inner0
		                                              : rhs.inner.inner1}});
	}
#endif

	template <usize I, typename Fn>
	VEG_INLINE VEG_CPP20(constexpr) void _emplace(Fn fn)
			VEG_NOEXCEPT_LIKE(VEG_FWD(fn)(UTag<I>{})) {
		auto& self_inner_inactive =
				(inner.tag_with_bit % 2U == 0) ? inner.inner1 : inner.inner0;

		// if this throws, object is not yet constructed, and lifetime does not
		// begin. we are still in a valid state
		mem::construct_at(
				mem::addressof(self_inner_inactive), UTag<I>{}, UTag<I>{}, VEG_FWD(fn));

		// invalid state here: two active members
		usize new_tag = I * 2U;
		usize old_tag = inner.tag_with_bit / 2U;
		usize old_low_bit = usize(inner.tag_with_bit % 2U == 1);
		usize new_low_bit = 1U - old_low_bit;
		inner.tag_with_bit = new_tag | new_low_bit;

		// dtor ends the lifetime of the object regardless of whether it throws
		// the call puts us in a valid state regardless of success (i hope)
		inner.destroy_inactive(old_tag);

		// defintely valid again, yay!
	}

	template <bool NoExcept, typename Fn>
	VEG_INLINE VEG_CPP14(constexpr) void assign(usize fn_tag, Fn fn)
			VEG_NOEXCEPT_IF(NoExcept) {
		auto& self_inner_inactive =
				(inner.tag_with_bit % 2U == 0) ? inner.inner1 : inner.inner0;

		auto& self_inner =
				(inner.tag_with_bit % 2U == 0) ? inner.inner0 : inner.inner1;

		usize self_tag = inner.tag_with_bit / 2U;

		if (self_tag == fn_tag) {
			// see above for justification

			internal::visit<
					void,
					VEG_ALL_OF(VEG_CONCEPT(nothrow_move_assignable<Storage<Ts>>)),
					sizeof...(Ts)>(
					self_tag,
					AssignFn<decltype(inner.inner0), Fn, Ts...>{self_inner, VEG_FWD(fn)});
		} else {
			internal::visit<
					void,
					VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Ts>)),
					sizeof...(Ts)>(
					fn_tag,
					CtorFn<decltype(inner.inner0), Fn>{self_inner_inactive, VEG_FWD(fn)});

			usize new_tag = fn_tag * 2U;
			usize old_tag = self_tag;
			usize old_low_bit = usize(inner.tag_with_bit % 2U == 1);
			usize new_low_bit = 1U - old_low_bit;
			inner.tag_with_bit = new_tag | new_low_bit;

			inner.destroy_inactive(old_tag);
		}
	}

	VEG_INLINE
	VEG_CPP14(constexpr)
	auto operator=(DoubleStorageCopyMove&& rhs) VEG_NOEXCEPT_IF(VEG_ALL_OF((
			VEG_CONCEPT(nothrow_move_assignable<Storage<Ts>>) &&
			VEG_CONCEPT(nothrow_move_constructible<Ts>)))) -> DoubleStorageCopyMove& {
		this->template assign<VEG_ALL_OF(
				(VEG_CONCEPT(nothrow_move_assignable<Storage<Ts>>) &&
		     VEG_CONCEPT(nothrow_move_constructible<Ts>)))>(
				inner.tag_with_bit / 2U == rhs.inner.tag_with_bit / 2U,
				rhs.inner.tag_with_bit / 2U,
				UwunionGetter<decltype(inner.inner0)&&, Ts...>{
						(rhs.inner.tag_with_bit % 2U == 0) ? rhs.inner.inner0
																							 : rhs.inner.inner1});
		return *this;
	}
	VEG_INLINE
	VEG_CPP14(constexpr)
	auto operator=(DoubleStorageCopyMove const& rhs) VEG_NOEXCEPT_IF(VEG_ALL_OF((
			VEG_CONCEPT(nothrow_copy_assignable<Storage<Ts>>) &&
			VEG_CONCEPT(nothrow_copy_constructible<Ts>)))) -> DoubleStorageCopyMove& {
		this->template assign<VEG_ALL_OF(
				(VEG_CONCEPT(nothrow_copy_assignable<Storage<Ts>>) &&
		     VEG_CONCEPT(nothrow_copy_constructible<Ts>)))>(
				inner.tag_with_bit / 2U == rhs.inner.tag_with_bit / 2U,
				rhs.inner.tag_with_bit / 2U,
				UwunionGetter<decltype(inner.inner0) const&, Ts...>{
						(rhs.inner.tag_with_bit % 2U == 0) ? rhs.inner.inner0
																							 : rhs.inner.inner1});
		return *this;
	}

	VEG_NODISCARD VEG_INLINE constexpr auto index() const VEG_NOEXCEPT -> usize {
		return inner.tag_with_bit / 2U;
	}
	VEG_INLINE constexpr auto get_union_ref() const VEG_NOEXCEPT
			-> RawUwunion<Ts...> const& {
		return (inner.tag_with_bit % 2U == 0) ? inner.inner0 : inner.inner1;
	}
};

template <usize... Is, typename... Ts>
struct NonTrivialUwunionImpl<
		needs_double_storage,
		meta::index_sequence<Is...>,
		Ts...> : DoubleStorageCopyMove<Ts...>,
						 meta::conditional_t<
								 VEG_ALL_OF(VEG_CONCEPT(move_constructible<Ts>)),
								 NoMoveCtor,
								 EmptyI<1312>>,
						 meta::conditional_t<
								 VEG_ALL_OF(VEG_CONCEPT(copy_constructible<Ts>)),
								 NoCopyCtor,
								 EmptyI<1313>>,
						 meta::conditional_t<
								 VEG_ALL_OF(
										 (VEG_CONCEPT(move_constructible<Ts>) &&
                      VEG_CONCEPT(move_assignable<Storage<Ts>>))),
								 NoMoveAssign,
								 EmptyI<1314>>,
						 meta::conditional_t<
								 VEG_ALL_OF(
										 (VEG_CONCEPT(copy_constructible<Ts>) &&
                      VEG_CONCEPT(copy_assignable<Storage<Ts>>))),
								 NoCopyAssign,
								 EmptyI<1315>> {
	using Base = DoubleStorageCopyMove<Ts...>;
	using Base::Base;
};

template <typename... Ts>
struct NonTrivialUwunionImplSelector {
	using type = NonTrivialUwunionImpl<
			!(VEG_ALL_OF(VEG_CONCEPT(nothrow_move_constructible<Ts>))) //
					? needs_double_storage
					: !(VEG_ALL_OF(VEG_CONCEPT(trivially_destructible<Ts>))) //
								? needs_dtor
								: needs_copy_move,
			meta::make_index_sequence<sizeof...(Ts)>,
			Ts...>;
};

template <typename... Ts>
using UwunionImpl = typename meta::conditional_t<
		VEG_ALL_OF(VEG_CONCEPT(trivially_copyable<Storage<Ts>>)),
		meta::type_identity<TrivialUwunionImpl<Ts...>>,
		NonTrivialUwunionImplSelector<Ts...>>::type;

template <typename Fn>
struct TaggedFn {
	Fn&& fn;
	VEG_INLINE constexpr auto
	operator()(void* /*unused*/) const&& VEG_DEDUCE_RET(VEG_FWD(fn)());
};
} // namespace _uwunion
} // namespace internal

template <typename... Ts>
struct Uwunion : private internal::_uwunion::UwunionImpl<Ts...> {
private:
	template <typename... Us>
	friend struct veg::Uwunion;
	template <typename, typename>
	friend struct internal::_uwunion::EmplaceWrapper;

	using Base = internal::_uwunion::UwunionImpl<Ts...>;
	using Base::Base;

public:
	VEG_EXPLICIT_COPY(Uwunion);

	VEG_TEMPLATE(
			(i64 I),
			requires(VEG_CONCEPT(move_constructible<ith<usize{I}, Ts...>>)),
			VEG_INLINE constexpr Uwunion,
			(/*itag*/, Fix<I>),
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
			(/*itag*/, Fix<I>),
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
			(/*itag*/, Fix<I>),
			(fn, Fn))
	VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_invocable<Fn>))->ith<usize{I}, Ts...>& {
		this->template _emplace<usize{I}>(
				internal::_uwunion::TaggedFn<Fn&&>{VEG_FWD(fn)});
		return const_cast<ith<usize{I}, Ts...>&>(
				internal::_uwunion::UwunionGetImpl<usize{I}>::get(this->get_union_ref())
						.inner);
	}

	VEG_TEMPLATE(
			(i64 I),
			requires(usize{I} < sizeof...(Ts)),
			VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto unwrap,
			(/*itag*/, Fix<I>)) &&
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
			(/*itag*/, Fix<I>),
			(/*unsafe_tag*/, Unsafe)) &&
			VEG_NOEXCEPT_IF(
					VEG_CONCEPT(nothrow_move_constructible<ith<usize{I}, Ts...>>))
					-> ith<usize{I}, Ts...> {
		meta::unreachable_if(usize{I} != usize(index()));
		return const_cast<ith<usize{I}, Ts...>&&>(
				internal::_uwunion::UwunionGetImpl<usize{I}>::get(this->get_union_ref())
						.inner);
	}

	using Base::index;

	void as_ref() const&& = delete;

	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto as_ref() & //
			VEG_NOEXCEPT -> Uwunion<Ts&...> {
		return internal::visit14<Uwunion<Ts&...>, true, sizeof...(Ts)>(
				usize(index()),
				internal::_uwunion::EmplaceWrapper<
						Uwunion<Ts&...>,
						internal::_uwunion::
								UwunionGetter<internal::_uwunion::RawUwunion<Ts...>&, Ts...>>{
						{const_cast<internal::_uwunion::RawUwunion<Ts...>&>(
								this->get_union_ref())},
						usize(index()),
				});
	}
	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto as_ref() const & //
			VEG_NOEXCEPT -> Uwunion<Ts const&...> {
		return internal::visit14<Uwunion<Ts const&...>, true, sizeof...(Ts)>(
				usize(index()),
				internal::_uwunion::EmplaceWrapper<
						Uwunion<Ts const&...>,
						internal::_uwunion::
								UwunionGetter<internal::_uwunion::RawUwunion<Ts...>&, Ts...>>{
						{const_cast<internal::_uwunion::RawUwunion<Ts...>&>(
								this->get_union_ref())},
						usize(index()),
				});
	}
	VEG_NODISCARD VEG_INLINE VEG_CPP14(constexpr) auto as_ref() && //
			VEG_NOEXCEPT -> Uwunion<Ts&&...> {
		return internal::visit14<Uwunion<Ts&&...>, true, sizeof...(Ts)>(
				usize(index()),
				internal::_uwunion::EmplaceWrapper<
						Uwunion<Ts&&...>,
						internal::_uwunion::
								UwunionGetter<internal::_uwunion::RawUwunion<Ts...>&, Ts...>>{
						{const_cast<internal::_uwunion::RawUwunion<Ts...>&>(
								this->get_union_ref())},
						usize(index()),
				});
	}
};
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_UWUNION_HPP_OHC4GK5JS */
