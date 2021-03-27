#ifndef VEG_ASSERT_HPP_VQDAJ2IBS
#define VEG_ASSERT_HPP_VQDAJ2IBS

#include "veg/internal/meta_int_fix.hpp"
#include "veg/internal/typedefs.hpp"
#include "veg/util/compare.hpp"
#include "veg/util/defer.hpp"
#include "veg/internal/byte_string_ref.hpp"
#include "veg/internal/simple_string.hpp"
#include "veg/internal/fmt.hpp"
#include "veg/internal/prologue.hpp"

#if HEDLEY_HAS_WARNING("-Woverloaded-shift-op-parentheses")
#define __VEG_IGNORE_SHIFT_PAREN_WARNING(Macro, ...)                           \
	HEDLEY_DIAGNOSTIC_PUSH _Pragma(                                              \
			"clang diagnostic ignored \"-Woverloaded-shift-op-parentheses\" ")       \
			Macro(__VA_ARGS__) HEDLEY_DIAGNOSTIC_POP
#else
#define __VEG_IGNORE_SHIFT_PAREN_WARNING(Macro, ...) Macro(__VA_ARGS__)
#endif

#if defined(__GNUC__)

#if defined(__clang__) || __cplusplus >= 201402L
#define __VEG_THIS_FUNCTION __PRETTY_FUNCTION__
#else
#define __VEG_THIS_FUNCTION ""
#endif

#elif defined(HEDLEY_MSVC_VERSION)
#define __VEG_THIS_FUNCTION __FUNCSIG__
#else
#define __VEG_THIS_FUNCTION __func__
#endif

namespace veg {
namespace abi {
inline namespace VEG_ABI_VERSION {
namespace internal {
void incr_counter() noexcept;
void decr_counter() noexcept;

[[noreturn]] void
on_assert_fail(long line, char_string_ref file, char_string_ref func);
void on_expect_fail(long line, char_string_ref file, char_string_ref func);
void set_assert_params1(char_string_ref op, string lhs, string rhs);
void set_assert_params2(
		char_string_ref expression, char_string_ref msg) noexcept;

struct cleanup { // NOLINT(cppcoreguidelines-special-member-functions)
	~cleanup() noexcept;
};
} // namespace internal
} // namespace VEG_ABI_VERSION
} // namespace abi

inline namespace VEG_ABI {
namespace internal {
namespace assert_ {

template <typename T>
auto to_string(T const& arg) -> abi::internal::string {
	abi::internal::string buf{};

	{
		abi::internal::incr_counter();
		auto&& cleanup = make::defer(abi::internal::decr_counter);
		(void)cleanup;
		veg::fmt::debug<meta::decay_t<T>>::to_string(buf, arg);
	}

	return buf;
}

template <typename T>
struct lhs_all_of_t {
	T const& lhs;

	template <typename U>
	HEDLEY_ALWAYS_INLINE void
	on_assertion_fail(U const& rhs, abi::internal::char_string_ref op_str) const {
		abi::internal::set_assert_params1(
				op_str, assert_::to_string(lhs), assert_::to_string(rhs));
	}

#define VEG_DISABLE(Op)                                                        \
	template <typename U>                                                        \
	void operator Op(U const& rhs) = delete

	VEG_DISABLE(&&);
	VEG_DISABLE(||);
	VEG_DISABLE(=);
	VEG_DISABLE(+=);
	VEG_DISABLE(-=);
	VEG_DISABLE(*=);
	VEG_DISABLE(/=);
	VEG_DISABLE(%=);
	VEG_DISABLE(<<=);
	VEG_DISABLE(>>=);
	VEG_DISABLE(&=);
	VEG_DISABLE(^=);
	VEG_DISABLE(|=);
	VEG_DISABLE(<<);
	VEG_DISABLE(>>);
	VEG_DISABLE(&);
	VEG_DISABLE(|);
	VEG_DISABLE(+);
	VEG_DISABLE(-);
	VEG_DISABLE(*);
	VEG_DISABLE(/);
	VEG_DISABLE(%);
	VEG_DISABLE(^);

	// if a VEG_ASSERT fails during the evaluation of the comparison result, the
	// behavior is undefined (potential recursion)

	VEG_TEMPLATE(
			typename U,
			requires(VEG_CONCEPT(equality_comparable_with<T, U>)),
			HEDLEY_ALWAYS_INLINE constexpr,
			auto
			operator==,
			(rhs, U const&))
	const->bool {
		return process_op(
				(veg::cmp::equal)(lhs, rhs), rhs, {" == ", sizeof(" == ") - 1});
	}
	VEG_TEMPLATE(
			typename U,
			requires(VEG_CONCEPT(equality_comparable_with<T, U>)),
			HEDLEY_ALWAYS_INLINE constexpr,
			auto
			operator!=,
			(rhs, U const&))
	const->bool {
		return process_op(
				!((veg::cmp::equal)(lhs, rhs)), rhs, {" != ", sizeof(" != ") - 1});
	}

	VEG_TEMPLATE(
			typename U,
			requires(VEG_CONCEPT(synth_three_way_comparable_with<T, U>)),
			HEDLEY_ALWAYS_INLINE constexpr,
			auto
			operator<,
			(rhs, U const&))
	const->bool {
		return process_op(
				(veg::cmp::synth_three_way)(lhs, rhs) < 0,
				rhs,
				{" < ", sizeof(" < ") - 1});
	}
	VEG_TEMPLATE(
			typename U,
			requires(VEG_CONCEPT(synth_three_way_comparable_with<T, U>)),
			HEDLEY_ALWAYS_INLINE constexpr,
			auto
			operator>,
			(rhs, U const&))
	const->bool {
		return process_op(
				(veg::cmp::synth_three_way)(lhs, rhs) > 0,
				rhs,
				{" > ", sizeof(" > ") - 1});
	}
	VEG_TEMPLATE(
			typename U,
			requires(VEG_CONCEPT(synth_three_way_comparable_with<T, U>)),
			HEDLEY_ALWAYS_INLINE constexpr,
			auto
			operator<=,
			(rhs, U const&))
	const->bool {
		return process_op(
				(veg::cmp::synth_three_way)(lhs, rhs) <= 0,
				rhs,
				{" <= ", sizeof(" <= ") - 1});
	}
	VEG_TEMPLATE(
			typename U,
			requires(VEG_CONCEPT(synth_three_way_comparable_with<T, U>)),
			HEDLEY_ALWAYS_INLINE constexpr,
			auto
			operator>=,
			(rhs, U const&))
	const->bool {
		return process_op(
				(veg::cmp::three_way)(lhs, rhs) >= 0,
				rhs,
				{" >= ", sizeof(" >= ") - 1});
	}

#undef VEG_DISABLE

	HEDLEY_ALWAYS_INLINE explicit constexpr
	operator bool() const { // NOLINT(hicpp-explicit-conversions)
		return process_bool(static_cast<bool>(lhs));
	}

	VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto process_bool(bool res) const
			-> bool {
		return (res ? void(0)
		            : abi::internal::set_assert_params1(
											abi::internal::empty_str, assert_::to_string(lhs), {})),
					 res;
	}
	template <typename U>
	VEG_NODISCARD HEDLEY_ALWAYS_INLINE constexpr auto
	process_op(bool res, U const& rhs, abi::internal::char_string_ref op) const
			-> bool {
		return (res ? void(0) : on_assertion_fail(rhs, op)), res;
	}
};

struct decomposer {
	template <typename T>
	HEDLEY_ALWAYS_INLINE constexpr auto operator<<(T const& lhs) const noexcept
			-> lhs_all_of_t<T> {
		return {lhs};
	}
};

#define __VEG_IMPL_ASSERT_IMPL(Callback, If_Fail, ...)                         \
	static_cast<void>(                                                           \
			(::veg::internal::assert_::decomposer{} << __VA_ARGS__)                  \
					? (void)(0)                                                          \
					: ((void)(::veg::abi::internal::cleanup{}),                          \
	           ::veg::abi::internal::set_assert_params2(                         \
								 ::veg::abi::internal::char_string_ref{                        \
										 static_cast<char const*>(#__VA_ARGS__),                   \
										 sizeof(#__VA_ARGS__) - 1},                                \
								 If_Fail),                                                     \
	           ::veg::abi::internal::Callback(                                   \
								 __LINE__,                                                     \
								 ::veg::abi::internal::char_string_ref{                        \
										 static_cast<char const*>(__FILE__),                       \
										 sizeof(__FILE__) - 1},                                    \
								 ::veg::abi::internal::char_string_ref{                        \
										 static_cast<char const*>(__VEG_THIS_FUNCTION),            \
										 sizeof(__VEG_THIS_FUNCTION) - 1})))

#define VEG_ASSERT_ELSE(...)                                                   \
	__VEG_IGNORE_SHIFT_PAREN_WARNING(                                            \
			__VEG_IMPL_ASSERT_IMPL, on_assert_fail, __VA_ARGS__)
#define VEG_EXPECT_ELSE(...)                                                   \
	__VEG_IGNORE_SHIFT_PAREN_WARNING(                                            \
			__VEG_IMPL_ASSERT_IMPL, on_expect_fail, __VA_ARGS__)

#define VEG_ASSERT(...)                                                        \
	__VEG_IGNORE_SHIFT_PAREN_WARNING(                                            \
			__VEG_IMPL_ASSERT_IMPL,                                                  \
			on_assert_fail,                                                          \
			(::veg::abi::internal::empty_str),                                       \
			__VA_ARGS__)

#define VEG_EXPECT(...)                                                        \
	__VEG_IGNORE_SHIFT_PAREN_WARNING(                                            \
			__VEG_IMPL_ASSERT_IMPL,                                                  \
			on_expect_fail,                                                          \
			(::veg::abi::internal::empty_str),                                       \
			__VA_ARGS__)

#define __VEG_IMPL_ALL_OF_1(Ftor, Decomposer, Callback, ...)                   \
	__VEG_IMPL_ALL_OF_2(                                                         \
			Ftor, Decomposer, Callback, __VEG_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

#define __VEG_IMPL_ASSERT_FTOR_EMPTY(_, Decomposer, Elem)                      \
	__VEG_IMPL_ASSERT_FTOR(                                                      \
			_,                                                                       \
			Decomposer,                                                              \
			(::veg::abi::internal::empty_str, __VEG_PP_REMOVE_PARENS(Elem)))

#define __VEG_IMPL_ASSERT_FTOR(_, Decomposer, Elem)                            \
	(::veg::internal::assert_::Decomposer{} << __VEG_PP_TAIL Elem)               \
			? true                                                                   \
			: ((void)(::veg::abi::internal::cleanup{}),                              \
	       ::veg::abi::internal::set_assert_params2(                             \
						 {static_cast<char const*>(                                        \
									__VEG_PP_STRINGIZE(__VEG_PP_TAIL Elem)),                     \
									sizeof(__VEG_PP_STRINGIZE(__VEG_PP_TAIL Elem)) - 1},         \
	            __VEG_PP_HEAD Elem),                                             \
						 false),

#define __VEG_IMPL_ALL_OF_2(Ftor, Decomposer, Callback, Seq)                   \
	(::veg::meta::all_of({__VEG_PP_SEQ_FOR_EACH(Ftor, Decomposer, Seq)})         \
	     ? (void)(0)                                                             \
	     : ::veg::abi::internal::Callback(                                       \
						 __LINE__,                                                         \
						 {static_cast<char const*>(__FILE__), sizeof(__FILE__) - 1},       \
						 {static_cast<char const*>(__VEG_THIS_FUNCTION),                   \
	            sizeof(__VEG_THIS_FUNCTION) - 1}))

#define VEG_ASSERT_ALL_OF(...)                                                 \
	__VEG_IGNORE_SHIFT_PAREN_WARNING(                                            \
			__VEG_IMPL_ALL_OF_1,                                                     \
			__VEG_IMPL_ASSERT_FTOR_EMPTY,                                            \
			decomposer,                                                              \
			on_assert_fail,                                                          \
			__VA_ARGS__)

#define VEG_EXPECT_ALL_OF(...)                                                 \
	__VEG_IGNORE_SHIFT_PAREN_WARNING(                                            \
			__VEG_IMPL_ALL_OF_1,                                                     \
			__VEG_IMPL_ASSERT_FTOR_EMPTY,                                            \
			decomposer,                                                              \
			on_expect_fail,                                                          \
			__VA_ARGS__)

#define VEG_ASSERT_ALL_OF_ELSE(...)                                            \
	__VEG_IGNORE_SHIFT_PAREN_WARNING(                                            \
			__VEG_IMPL_ALL_OF_1,                                                     \
			__VEG_IMPL_ASSERT_FTOR,                                                  \
			decomposer,                                                              \
			on_assert_fail,                                                          \
			__VA_ARGS__)

#define VEG_EXPECT_ALL_OF_ELSE(...)                                            \
	__VEG_IGNORE_SHIFT_PAREN_WARNING(                                            \
			__VEG_IMPL_ALL_OF_1,                                                     \
			__VEG_IMPL_ASSERT_FTOR,                                                  \
			decomposer,                                                              \
			on_expect_fail,                                                          \
			__VA_ARGS__)

} // namespace assert_
} // namespace internal
} // namespace VEG_ABI
} // namespace veg

#include "veg/internal/epilogue.hpp"
#endif /* end of include guard VEG_ASSERT_HPP_VQDAJ2IBS */

#ifdef VEG_DEBUG_ASSERT
#undef VEG_DEBUG_ASSERT
#endif
#ifdef VEG_DEBUG_EXPECT
#undef VEG_DEBUG_EXPECT
#endif
#ifdef VEG_DEBUG_ASSERT_ELSE
#undef VEG_DEBUG_ASSERT_ELSE
#endif
#ifdef VEG_DEBUG_EXPECT_ELSE
#undef VEG_DEBUG_EXPECT_ELSE
#endif
#ifdef VEG_DEBUG_ASSERT_ALL_OF
#undef VEG_DEBUG_ASSERT_ALL_OF
#endif
#ifdef VEG_DEBUG_EXPECT_ALL_OF
#undef VEG_DEBUG_EXPECT_ALL_OF
#endif
#ifdef VEG_DEBUG_ASSERT_ALL_OF_ELSE
#undef VEG_DEBUG_ASSERT_ALL_OF_ELSE
#endif
#ifdef VEG_DEBUG_EXPECT_ALL_OF_ELSE
#undef VEG_DEBUG_EXPECT_ALL_OF_ELSE
#endif

#ifdef NDEBUG
#define VEG_DEBUG_ASSERT(...) ((void)(0))
#define VEG_DEBUG_EXPECT(...) ((void)(0))
#define VEG_DEBUG_ASSERT_ELSE(Message, ...) ((void)(0))
#define VEG_DEBUG_EXPECT_ELSE(Message, ...) ((void)(0))
#define VEG_DEBUG_ASSERT_ALL_OF(...) ((void)(0))
#define VEG_DEBUG_EXPECT_ALL_OF(...) ((void)(0))
#define VEG_DEBUG_ASSERT_ALL_OF_ELSE(...) ((void)(0))
#define VEG_DEBUG_EXPECT_ALL_OF_ELSE(...) ((void)(0))
#else
#define VEG_DEBUG_ASSERT(...) VEG_ASSERT(__VA_ARGS__)
#define VEG_DEBUG_EXPECT(...) VEG_EXPECT(__VA_ARGS__)
#define VEG_DEBUG_ASSERT_ELSE(Message, ...)                                    \
	VEG_ASSERT_ELSE(Message, __VA_ARGS__)
#define VEG_DEBUG_EXPECT_ELSE(Message, ...)                                    \
	VEG_EXPECT_ELSE(Message, __VA_ARGS__)
#define VEG_DEBUG_ASSERT_ALL_OF(...) VEG_ASSERT_ALL_OF(__VA_ARGS__)
#define VEG_DEBUG_EXPECT_ALL_OF(...) VEG_EXPECT_ALL_OF(__VA_ARGS__)
#define VEG_DEBUG_ASSERT_ALL_OF_ELSE(...) VEG_ASSERT_ALL_OF_ELSE(__VA_ARGS__)
#define VEG_DEBUG_EXPECT_ALL_OF_ELSE(...) VEG_EXPECT_ALL_OF_ELSE(__VA_ARGS__)
#endif
