#include <veg/internal/parse.hpp>
#include <cstdio>
#include <fmt/core.h>

namespace veg {
namespace _detail {
namespace type_parse {
void function_decl_to_file(
		std::FILE* f, _detail::type_parse::FunctionDecl decl) noexcept;
} // namespace type_parse
} // namespace _detail
} // namespace veg

#define PRINT_FN() ::fmt::print("{}\n", __PRETTY_FUNCTION__)

namespace test_cases {
void plain_fn() {
	PRINT_FN();
}
auto usi() -> unsigned short int {
	PRINT_FN();
	return {};
}
auto ulli() -> unsigned long long int {
	PRINT_FN();
	return {};
}
auto ssi() -> signed short int {
	PRINT_FN();
	return {};
}
auto slli() -> signed long long int {
	PRINT_FN();
	return {};
}

auto uc() -> unsigned char {
	PRINT_FN();
	return {};
}
auto sc() -> signed char {
	PRINT_FN();
	return {};
}
auto c() -> char {
	PRINT_FN();
	return {};
}

struct S {
	void plain_fn() const { (void)this, PRINT_FN(); }
	static void plain_fn_static() { PRINT_FN(); }
};

template <typename T>
struct TplS {
	void plain_fn(int /*unused*/ = {}) const { PRINT_FN(); }
	auto plain_fn_trailing() const -> decltype(T()) { PRINT_FN(); }
	template <typename U>
	void tpl_fn_trailing() const {
		PRINT_FN();
	}

	template <typename U>
	struct DtorInner /* NOLINT */ {
		~DtorInner() { PRINT_FN(); }
	};
	template <typename U>
	struct CtorInner {
		CtorInner() { PRINT_FN(); }
	};
};

struct A {
	friend void operator+(A /*unused*/, A /*unused*/) { PRINT_FN(); }
	void operator-(A /*unused*/) const { PRINT_FN(); }
	void operator[](int /*unused*/) const { PRINT_FN(); }
	void operator()(int /*unused*/) const { PRINT_FN(); }
	void operator&&(A /*unused*/) const { PRINT_FN(); }
};
} // namespace test_cases

auto main() -> int {
	using namespace veg::_detail::type_parse;
	using namespace veg;
	using Uwunion = Entity::Uwunion;

	auto decl = parse_function_decl(
			{from_literal,
	     "def::abc<::veg::internal::meta_::discard_1st< "
	     "::veg::meta::enable_if_t<((::veg::concepts::movable<T>) && "
	     "::veg::meta::bool_constant<(_ == 0)>::value)>, T>> "
	     "veg::Option<int>::unwrap() && [T = int, _ = 0]"});
	dbg(decl);
	function_decl_to_file(stderr, VEG_FWD(decl));
	std::fputc('\n', stderr);

	VEG_ASSERT(
			dbg(greedy_parse_entity(
					StrView{from_literal, "const int volatile * const & "})[0_c]) ==
			Entity{
					Uwunion{
							3_c,
							Pointer{
									box(Entity{
											Uwunion{
													3_c,
													Pointer{
															box(Entity{
																	{0_c, {{from_literal, "int"}}},
																	CvQual::CONST_VOLATILE,
															}),
															RefQual::POINTER},
											},
											CvQual::CONST,
									}),
									RefQual::LVALUE},
					},
					CvQual::NONE,
			});

	{
		test_cases::plain_fn();
		constexpr StrView tests[] = {
				{from_literal, "void test_cases::plain_fn() "}};
	}
	{
		test_cases::S{}.plain_fn();
		constexpr StrView tests[] = {
				{from_literal, "void test_cases::S::plain_fn() const"},
				{from_literal, "void test_cases::S::plain_fn() const"}};

		for (auto test : tests) {
			VEG_ASSERT(
					greedy_parse_entity(test)[0_c] ==
					Entity{
							Uwunion{
									0_c,
									EntityName{{from_literal, "void"}},
							},
							CvQual::NONE,
					});
		}
	}
	{
		test_cases::S::plain_fn_static();
		constexpr StrView tests[] = {
				{from_literal, "static void test_cases::S::plain_fn_static()"},
				{from_literal, "static void test_cases::S::plain_fn_static()"}};

		for (auto test : tests) {
			VEG_ASSERT(
					greedy_parse_entity(test)[0_c] ==
					Entity{
							Uwunion{
									0_c,
									EntityName{{from_literal, "static"}},
							},
							CvQual::NONE,
					});
		}
	}
	{
		test_cases::TplS<void>{}.plain_fn();
		constexpr StrView tests[] = {
				{from_literal,
		     "void test_cases::TplS<T>::plain_fn() const [with T = void]"},
				{from_literal,
		     "void test_cases::TplS<void>::plain_fn() const [T = void]"}};

		for (auto test : tests) {
			VEG_ASSERT(
					greedy_parse_entity(test)[0_c] ==
					Entity{
							Uwunion{
									0_c,
									EntityName{{from_literal, "void"}},
							},
							CvQual::NONE,
					});
		}
	}
	{
		test_cases::TplS<void>{}.plain_fn_trailing();
		constexpr StrView tests[] = {
				{from_literal,
		     "decltype (T()) test_cases::TplS<T>::plain_fn_trailing() const [with"
		     " T = void; decltype (T()) = void]"},
				{from_literal,
		     "<dependent type> test_cases::TplS<void>::plain_fn_trailing() const "
		     "[T = void]"}};

		{
			auto test = tests[0];
			VEG_ASSERT(
					greedy_parse_entity(test)[0_c] ==
					Entity{
							Uwunion{
									0_c,
									EntityName{{from_literal, "decltype (T())"}},
							},
							CvQual::NONE,
					});
		}
		{
			auto test = tests[1];
			VEG_ASSERT(
					greedy_parse_entity(test)[0_c] ==
					Entity{
							Uwunion{
									0_c,
									EntityName{{from_literal, "<dependent type>"}},
							},
							CvQual::NONE,
					});
		}
	}
	{
		auto l = [] {};
		test_cases::TplS<void>{}.tpl_fn_trailing<decltype(l)>();
		constexpr StrView tests[] = {
				{from_literal,
		     "void test_cases::TplS<T>::tpl_fn_trailing() const [with U = "
		     "main()::<lambda()>; T = void]"},
				{from_literal,
		     "void test_cases::TplS<void>::tpl_fn_trailing() const [T = void, U ="
		     "(lambda at b.cpp:360:12)]"}};

		for (auto test : tests) {
			VEG_ASSERT(
					greedy_parse_nested_entity(test)[0_c].kind[0_c].name ==
					StrView{from_literal, "void"});
		}
	}
	{
		test_cases::TplS<void>::CtorInner<int>{};
		constexpr StrView tests[] = {
				{from_literal,
		     "test_cases::TplS<T>::CtorInner<U>::CtorInner() [with U = int; T = "
		     "void]"},
				{from_literal,
		     "test_cases::TplS<void>::CtorInner<int>::CtorInner() [T = void, U = "
		     "int]"}};
	}
	{
		(void)test_cases::TplS<void>::DtorInner<int>{};
		constexpr StrView tests[] = {
				{from_literal,
		     "test_cases::TplS<T>::DtorInner<U>::~DtorInner() [with U = int; T = "
		     "void]"},
				{from_literal,
		     "test_cases::TplS<void>::DtorInner<int>::~DtorInner() [T = void, U ="
		     "int]"}};
	}

	{
		test_cases::ulli();
		constexpr StrView tests[] = {
				{from_literal, "long long unsigned int test_cases::ulli()"},
				{from_literal, "unsigned long long test_cases::ulli()"}};

		{
			auto test = tests[0];
			VEG_ASSERT(
					greedy_parse_nested_entity(test)[0_c].kind.unwrap(0_c).name ==
					StrView{from_literal, "long long unsigned int"});
		}
		{
			auto test = tests[1];
			VEG_ASSERT(
					greedy_parse_nested_entity(test)[0_c].kind.unwrap(0_c).name ==
					StrView{from_literal, "unsigned long long"});
		}
	}
	{
		test_cases::slli();
		constexpr StrView tests[] = {
				{from_literal, "long long int test_cases::slli()"},
				{from_literal, "long long test_cases::slli()"}};
	}
	{
		test_cases::ssi();
		constexpr StrView tests[] = {
				{from_literal, "short int test_cases::ssi()"},
				{from_literal, "short test_cases::ssi()"}};
	}
	{
		test_cases::usi();
		constexpr StrView tests[] = {
				{from_literal, "short unsigned int test_cases::usi()"},
				{from_literal, "unsigned short test_cases::usi()"}};
	}
	{
		test_cases::uc();
		constexpr StrView tests[] = {
				{from_literal, "unsigned char test_cases::uc()"},
				{from_literal, "unsigned char test_cases::uc()"}};
	}
	{
		test_cases::sc();
		constexpr StrView tests[] = {
				{from_literal, "signed char test_cases::sc()"},
				{from_literal, "signed char test_cases::sc()"}};
	}
	{
		test_cases::c();
		constexpr StrView tests[] = {
				{from_literal, "char test_cases::c()"},
				{from_literal, "char test_cases::c()"}};
	}

	{
		test_cases::A{} + test_cases::A{};
		test_cases::A{} - test_cases::A{};
		test_cases::A{} && test_cases::A{};
		test_cases::A{}[0];
		test_cases::A{}(0);
	}
}
