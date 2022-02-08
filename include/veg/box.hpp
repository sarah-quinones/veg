#ifndef VEG_BOX_HPP_HUJDLY6PS
#define VEG_BOX_HPP_HUJDLY6PS

#include "veg/internal/delete_special_members.hpp"
#include "veg/type_traits/constructible.hpp"
#include "veg/memory/alloc.hpp"
#include "veg/tuple.hpp"
#include "veg/util/assert.hpp"
#include "veg/internal/prologue.hpp"

namespace veg {
namespace _detail {
namespace _mem {
template <typename T>
struct UniquePtr {
	T* inner = nullptr;
	UniquePtr() = default;
	UniquePtr(UniquePtr&& rhs) noexcept : inner{rhs.inner} { rhs.inner = {}; }
	VEG_INLINE UniquePtr(UniquePtr const& /*unused*/) noexcept {}
	auto operator=(UniquePtr const&) -> UniquePtr& = delete;
	auto operator=(UniquePtr&&) -> UniquePtr& = delete;
	~UniquePtr() = default;
};

template <typename T, typename A>
struct BoxAlloc : Tuple<A, UniquePtr<T>> {
	BoxAlloc() = default;

	using Tuple<A, UniquePtr<T>>::Tuple;

	BoxAlloc(BoxAlloc&&) = default;
	BoxAlloc(BoxAlloc const&) = default;
	auto operator=(BoxAlloc&&) -> BoxAlloc& = delete;
	auto operator=(BoxAlloc const&) -> BoxAlloc& = delete;

	VEG_INLINE ~BoxAlloc() {
		if ((*this)[1_c].inner != nullptr) {
			mem::Alloc<A>::dealloc(
					mut((*this)[0_c]),
					static_cast<void*>((*this)[1_c].inner),
					mem::Layout{sizeof(T), alignof(T)});
		}
	}
};

template <typename T>
struct DerefCopyFn {
	T const* ptr;
	VEG_INLINE auto
	operator()() const&& VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_copyable<T>)) -> T {
		return T(*ptr);
	}
};
} // namespace _mem
} // namespace _detail

namespace mem {
template <
		typename T,
		typename A = SystemAlloc,
		bool NoThrowCopy = VEG_CONCEPT(nothrow_copyable<T>) &&
                       VEG_CONCEPT(nothrow_copy_assignable<T>)>
struct BoxIncomplete {
	_detail::_mem::BoxAlloc<T, A> _;

	VEG_INLINE constexpr auto alloc_ref() const noexcept -> Ref<A> {
		return ref(_[0_c]);
	}
	VEG_INLINE VEG_CPP14(constexpr) auto alloc_mut(Unsafe /*tag*/) noexcept
			-> RefMut<A> {
		return mut(_[0_c]);
	}
	VEG_INLINE constexpr auto data_ref() const noexcept -> Ref<T const*> {
		return ref(*static_cast<T const* const*>(mem::addressof(_[1_c].inner)));
	}
	VEG_INLINE VEG_CPP14(constexpr) auto data_mut(Unsafe /*tag*/) noexcept
			-> RefMut<T*> {
		return mut(_[1_c].inner);
	}

	VEG_INLINE constexpr auto ptr() const noexcept -> T const* {
		return _[1_c].inner;
	}
	VEG_INLINE VEG_CPP14(constexpr) auto ptr_mut() noexcept -> T* {
		return _[1_c].inner;
	}

	VEG_INLINE constexpr auto operator*() const noexcept -> T const& {
		VEG_ASSERT(ptr() != nullptr);
		return *ptr();
	}
	VEG_INLINE VEG_CPP14(constexpr) auto operator*() noexcept -> T& {
		VEG_ASSERT(ptr() != nullptr);
		return *ptr_mut();
	}
	VEG_INLINE constexpr auto operator->() const noexcept -> T const* {
		VEG_ASSERT(ptr() != nullptr);
		return ptr();
	}
	VEG_INLINE VEG_CPP14(constexpr) auto operator->() noexcept -> T* {
		VEG_ASSERT(ptr() != nullptr);
		return ptr_mut();
	}

	template <int _>
	VEG_INLINE
	BoxIncomplete(Unsafe /*unsafe*/, _::FromRawParts<_> /*tag*/, A alloc, T* ptr)
			VEG_NOEXCEPT : _{
												 tuplify,
												 VEG_FWD(alloc),
												 _detail::_mem::UniquePtr<T>{},
										 } {
		data_mut(unsafe).get() = VEG_FWD(ptr);
	}

	template <int _>
	BoxIncomplete(_::FromAlloc<_> /*tag*/, A alloc) VEG_NOEXCEPT
			: _{
						tuplify,
						VEG_FWD(alloc),
						_detail::_mem::UniquePtr<T>{},
				} {}

	template <int _, typename U = T>
	BoxIncomplete(_::FromAllocAndVaue<_> /*tag*/, A alloc, T value)
			VEG_NOEXCEPT_IF(
					VEG_CONCEPT(nothrow_movable<U>) &&
					VEG_CONCEPT(alloc::nothrow_alloc<A>))
			: BoxIncomplete{from_alloc, VEG_FWD(alloc)} {
		mem::Layout l{
				sizeof(T),
				alignof(T),
		};

		_detail::_mem::ManagedAlloc<A> block{
				mem::Alloc<A>::alloc(this->alloc_mut(unsafe), l).data,
				l,
				this->alloc_mut(unsafe),
		};

		mem::construct_at(static_cast<T*>(block.data), VEG_FWD(value));

		this->data_mut(unsafe).get() = static_cast<T*>(block.data);
		block.data = nullptr;
	}

	BoxIncomplete() = default;
	BoxIncomplete(BoxIncomplete&&) = default;

	explicit BoxIncomplete(BoxIncomplete const& rhs) VEG_NOEXCEPT_IF(
			VEG_CONCEPT(nothrow_copyable<A>) &&
			VEG_CONCEPT(alloc::nothrow_alloc<A>) && NoThrowCopy)
			: _{
						InPlace<Tuplify>{},
						_detail::CopyFn<A>{rhs.alloc_ref().get()},
						_detail::DefaultFn<_detail::_mem::UniquePtr<T>>{},
				} {
		if (rhs.ptr() == nullptr) {
			return;
		}

		mem::Layout l{
				sizeof(T),
				alignof(T),
		};

		_detail::_mem::ManagedAlloc<A> block{
				mem::Alloc<A>::alloc(this->alloc_mut(unsafe), l).data,
				l,
				this->alloc_mut(unsafe),
		};

		mem::construct_at(static_cast<T*>(block.data), *rhs.ptr());
		this->data_mut(unsafe).get() = static_cast<T*>(block.data);
		block.data = nullptr;
	}

	VEG_INLINE auto operator=(BoxIncomplete&& rhs) noexcept -> BoxIncomplete& {
		{ auto cleanup = static_cast<BoxIncomplete&&>(*this); }
		this->alloc_mut(unsafe).get() =
				static_cast<A&&>(rhs.alloc_mut(unsafe).get());
		this->data_mut(unsafe).get() = rhs.ptr_mut();

		rhs.data_mut(unsafe).get() = nullptr;
		return *this;
	}

	auto operator=(BoxIncomplete const& rhs) VEG_NOEXCEPT_IF(
			VEG_CONCEPT(alloc::nothrow_alloc<A>) &&
			VEG_CONCEPT(nothrow_copy_assignable<A>) && NoThrowCopy)
			-> BoxIncomplete& {
		static_assert(
				NoThrowCopy == (VEG_CONCEPT(nothrow_copyable<T>) &&
		                    VEG_CONCEPT(nothrow_copy_assignable<T>)),
				".");
		if (this != mem::addressof(rhs)) {
			if (cmp::eq(this->alloc_ref(), rhs.alloc_ref()) && //
			    ptr() != nullptr && rhs.ptr() != nullptr) {
				alloc_mut(unsafe).get() = rhs.alloc_ref().get();
				*ptr_mut() = *rhs.ptr();
			} else {
				*this = BoxIncomplete(rhs);
			}
		}
		return *this;
	}

private:
	void _destroy(T* ptr) noexcept { mem::destroy_at(ptr); }

public:
	VEG_INLINE ~BoxIncomplete() {
		static_assert(VEG_CONCEPT(nothrow_destructible<T>), ".");
		static_assert(VEG_CONCEPT(alloc::nothrow_dealloc<A>), ".");
		auto ptr = this->ptr_mut();
		if (ptr != nullptr) {
			this->_destroy(ptr);
		}
	}
};

VEG_TEMPLATE(
		(typename LT, typename RT, typename LA, typename RA),
		requires(VEG_CONCEPT(eq<LT, RT>)),
		VEG_NODISCARD constexpr auto
		operator==,
		(lhs, BoxIncomplete<LT, LA> const&),
		(rhs, BoxIncomplete<RT, RA> const&))
VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_eq<LT, RT>))->bool {
	return (lhs.ptr() == nullptr || rhs.ptr() == nullptr)
	           ? (lhs.ptr() == nullptr && rhs.ptr() == nullptr)
	           : (*lhs.ptr() == *rhs.ptr());
}
} // namespace mem
template <typename T, typename A = mem::SystemAlloc>
struct Box
		: meta::conditional_t<
					VEG_CONCEPT(nothrow_copyable<T>) && VEG_CONCEPT(nothrow_copyable<A>),
					_detail::EmptyI<13>,
					_detail::NoCopyCtor>,
			meta::conditional_t<
					VEG_CONCEPT(nothrow_copy_assignable<T>) &&
							VEG_CONCEPT(nothrow_copy_assignable<A>),
					_detail::EmptyI<12>,
					_detail::NoCopyAssign>,

			mem::BoxIncomplete<T, A> {
	using mem::BoxIncomplete<T, A>::BoxIncomplete;
	VEG_EXPLICIT_COPY(Box);
};

namespace _detail {
namespace _mem {
struct OrdBoxI {
	VEG_TEMPLATE(
			(typename LT, typename RT, typename LA, typename RA),
			requires(VEG_CONCEPT(ord<LT, RT>)),
			VEG_NODISCARD static VEG_CPP14(constexpr) auto cmp,
			(lhs, Ref<mem::BoxIncomplete<LT, LA>>),
			(rhs, Ref<mem::BoxIncomplete<RT, RA>>))
	VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_ord<LT, RT>))->cmp::Ordering {
		if (lhs.get().ptr() == nullptr && rhs.get().ptr() == nullptr) {
			return cmp::Ordering::equal;
		}
		if (lhs.get().ptr() == nullptr) {
			return cmp::Ordering::less;
		}
		if (rhs.get().ptr() == nullptr) {
			return cmp::Ordering::greater;
		}
		return cmp::cmp( //
				ref(*lhs.get().ptr()),
				ref(*rhs.get().ptr()));
	}
};
struct OrdBox {
	VEG_TEMPLATE(
			(typename LT, typename RT, typename LA, typename RA),
			requires(VEG_CONCEPT(ord<LT, RT>)),
			VEG_NODISCARD static VEG_CPP14(constexpr) auto cmp,
			(lhs, Ref<Box<LT, LA>>),
			(rhs, Ref<Box<RT, RA>>))
	VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_ord<LT, RT>))->cmp::Ordering {
		return OrdBoxI::cmp( //
				ref(static_cast<mem::BoxIncomplete<LT, LA> const&>(lhs.get())),
				ref(static_cast<mem::BoxIncomplete<RT, RA> const&>(rhs.get())));
	}
};
struct DbgBoxI {
	template <typename T>
	static void to_string_impl(fmt::BufferMut out, T const* ptr) noexcept {
		if (ptr != nullptr) {
			out.append_literal(u8"some(");
			fmt::dbg_to(VEG_FWD(out), ref(*ptr));
			out.append_literal(u8")");
		} else {
			out.append_literal(u8"none");
		}
	}

	template <typename T, typename A>
	static void
	to_string(fmt::BufferMut out, Ref<mem::BoxIncomplete<T, A>> r) noexcept {
		to_string_impl(VEG_FWD(out), r.get().ptr());
	}
};
struct DbgBox {
	template <typename T, typename A>
	static void to_string(fmt::BufferMut out, Ref<Box<T, A>> r) noexcept {
		DbgBoxI::to_string(
				VEG_FWD(out),
				ref(static_cast<mem::BoxIncomplete<T, A> const&>(r.get())));
	}
};
} // namespace _mem
} // namespace _detail

template <typename T, typename A>
struct cpo::is_trivially_relocatable<mem::BoxIncomplete<T, A>>
		: cpo::is_trivially_relocatable<A> {};
template <typename T, typename A>
struct cpo::is_trivially_constructible<mem::BoxIncomplete<T, A>>
		: cpo::is_trivially_constructible<A> {};

template <typename T, typename A>
struct cpo::is_trivially_relocatable<Box<T, A>>
		: cpo::is_trivially_relocatable<A> {};
template <typename T, typename A>
struct cpo::is_trivially_constructible<Box<T, A>>
		: cpo::is_trivially_constructible<A> {};

template <typename LT, typename RT, typename LA, typename RA>
struct cmp::Ord<mem::BoxIncomplete<LT, LA>, mem::BoxIncomplete<RT, RA>>
		: _detail::_mem::OrdBoxI {};
template <typename LT, typename RT, typename LA, typename RA>
struct cmp::Ord<Box<LT, LA>, Box<RT, RA>> : _detail::_mem::OrdBox {};

template <typename T, typename A>
struct fmt::Debug<mem::BoxIncomplete<T, A>> : _detail::_mem::DbgBoxI {};
template <typename T, typename A>
struct fmt::Debug<Box<T, A>> : _detail::_mem::DbgBox {};

namespace nb {
struct box {
	template <typename T>
	VEG_INLINE auto operator()(T val) const
			VEG_NOEXCEPT_IF(VEG_CONCEPT(nothrow_movable<T>)) -> Box<T> {
		return {
				from_alloc_and_value,
				mem::SystemAlloc{},
				VEG_FWD(val),
		};
	}
};
struct box_with_alloc {
	VEG_TEMPLATE(
			(typename A, typename T),
			requires(
					VEG_CONCEPT(nothrow_movable<A>) &&         //
					VEG_CONCEPT(nothrow_move_assignable<A>) && //
					VEG_CONCEPT(alloc::alloc<A>)),
			VEG_INLINE auto
			operator(),
			(alloc, A),
			(val, T))
	VEG_NOEXCEPT_IF(
			VEG_CONCEPT(alloc::nothrow_alloc<A>) && //
			VEG_CONCEPT(nothrow_movable<T>))
			->Box<T, A> {
		return {
				from_alloc_and_value,
				VEG_FWD(alloc),
				VEG_FWD(val),
		};
	}
};
} // namespace nb
VEG_NIEBLOID(box);
VEG_NIEBLOID(box_with_alloc);
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_BOX_HPP_HUJDLY6PS */
