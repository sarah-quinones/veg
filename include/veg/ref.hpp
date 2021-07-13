#ifndef VEG_REF_HPP_VQCOSD7RS
#define VEG_REF_HPP_VQCOSD7RS

#include "veg/type_traits/tags.hpp"
#include "veg/memory/address.hpp"

namespace veg {

template <typename T>
struct Ref {
private:
	T const* ptr{};
	VEG_INLINE constexpr Ref() = default;

public:
	VEG_TEMPLATE(
			(typename U = T),
			requires(VEG_CONCEPT(same<T, U>)),
			VEG_NODISCARD VEG_INLINE constexpr Ref,
			(/*tag*/, AsRef),
			(ref, U const&))
	noexcept
			: ptr{mem::addressof(ref)} {}
	VEG_NODISCARD VEG_INLINE constexpr auto get() const noexcept -> T const& {
		return *ptr;
	}
	VEG_NODISCARD VEG_INLINE constexpr auto operator*() const noexcept
			-> T const& {
		return *ptr;
	}
	VEG_NODISCARD VEG_INLINE constexpr auto operator->() const noexcept
			-> T const* {
		return ptr;
	}
};

template <typename T>
struct RefMut {
private:
	T* ptr;
	VEG_INLINE constexpr RefMut() = default;

public:
	VEG_TEMPLATE(
			(typename U = T),
			requires(VEG_CONCEPT(same<T, meta::unref_t<U>>)),
			VEG_NODISCARD VEG_INLINE constexpr RefMut,
			(/*tag*/, AsMut),
			(ref, U&&))
	noexcept
			: ptr{mem::addressof(ref)} {}

	VEG_NO_COPY(RefMut);

	VEG_INLINE constexpr auto as_const() const noexcept -> Ref<T> {
		return Ref<T>{
				AsRef{},
				*ptr,
		};
	}
	VEG_NODISCARD VEG_INLINE constexpr auto get() const noexcept -> T& {
		return *ptr;
	}
	VEG_NODISCARD VEG_INLINE constexpr auto operator*() const noexcept -> T& {
		return *ptr;
	}
	VEG_NODISCARD VEG_INLINE constexpr auto operator->() const noexcept -> T* {
		return ptr;
	}
};

template <typename T>
struct RefOnce {
private:
	T* ptr;
	constexpr RefOnce() = default;

public:
	VEG_TEMPLATE(
			(typename U = T),
			requires(VEG_CONCEPT(same<T, U>)),
			constexpr RefOnce,
			(/*tag*/, AsRefOnce),
			(ref, U&&))
	noexcept
			: ptr{mem::addressof(ref)} {}

	VEG_NO_COPY(RefOnce);

	constexpr auto as_ref() const noexcept -> Ref<T> {
		return Ref<T>{
				AsRef{},
				*ptr,
		};
	}
	constexpr auto as_mut() const noexcept -> RefMut<T> {
		return RefMut<T>{
				AsMut{},
				*ptr,
		};
	}
	constexpr auto get() const&& noexcept -> T&& {
		return static_cast<T&&>(*ptr);
	}
};

namespace nb {
struct deref {
	template <typename T>
	constexpr auto operator()(Ref<T> r) const noexcept -> T const& {
		return r.get();
	}
};
struct deref_mut {
	template <typename T>
	constexpr auto operator()(RefMut<T> r) const noexcept -> T& {
		return r.get();
	}
};

struct ref {
	template <typename T>
	constexpr auto operator()(T const& r) const noexcept -> Ref<T> {
		return {
				as_ref,
				r,
		};
	}
};
struct mut {
	VEG_TEMPLATE(
			typename T,
			requires(!VEG_CONCEPT(const_type<T>)),
			constexpr auto
			operator(),
			(r, T&&))
	const noexcept->RefMut<meta::uncvref_t<T>> {
		return {
				as_mut,
				r,
		};
	}
};
struct ref_once {
	VEG_TEMPLATE(
			typename T,
			requires(
					!VEG_CONCEPT(const_type<T>) && //
					!VEG_CONCEPT(lvalue_reference<T>)),
			constexpr auto
			operator(),
			(r, T&&))
	const noexcept->RefOnce<T> {
		return {
				as_ref_once,
				VEG_FWD(r),
		};
	}
};
} // namespace nb
VEG_NIEBLOID(ref);
VEG_NIEBLOID(mut);
VEG_NIEBLOID(ref_once);
} // namespace veg

#endif /* end of include guard VEG_REF_HPP_VQCOSD7RS */
