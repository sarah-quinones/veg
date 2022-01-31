#include "static_assert.hpp"
#include <limits>
#include <veg/tuple.hpp>
#include <utility>
#include <doctest.h>
#include "veg/memory/placement.hpp"
#include "veg/functional/overload.hpp"
#include "veg/functional/indexed.hpp"
#include "veg/functional/copy_fn.hpp"
#include "veg/functional/bind.hpp"
#include "veg/internal/prologue.hpp"

#define FWD VEG_FWD
#define ASSERT_SAME(...) STATIC_ASSERT(::std::is_same<__VA_ARGS__>::value)

using namespace veg::literals;
using namespace veg::tags;
using veg::fn::bind_back_once;
using veg::tuplify;

template <typename>
void get() = delete;

TEST_CASE("tuple: adl_get") {
	veg::Tuple<int, float> t{};
	veg::unused(get<0>(t));
	STATIC_ASSERT(VEG_CONCEPT(same<decltype(get<0>(t)), int&>));
	STATIC_ASSERT(VEG_CONCEPT(
			same<
					decltype(get<0>(static_cast<veg::Tuple<int, float> const&>(t))),
					int const&>));
	STATIC_ASSERT(VEG_CONCEPT(
			same<decltype(get<0>(static_cast<veg::Tuple<int, float>&&>(t))), int&&>));
}

TEST_CASE("tuple: all") {
	using namespace veg;
	using veg::clone;

	auto tup = tuplify(1, 'c', true);
	veg::Tuple<int, char&&, char, bool&, bool const&> tup_ref{
			tuplify,
			1,
			static_cast<char&&>(tup[1_c]),
			'c',
			nb::get<2>{}(tup),
			nb::get<2>{}(tup)};
	{
		STATIC_ASSERT(veg::cpo::is_trivially_relocatable<decltype(tup)>::value);
		STATIC_ASSERT(VEG_CONCEPT(trivially_copyable<decltype(tup)>));

		STATIC_ASSERT(!VEG_CONCEPT(copy_assignable<veg::Tuple<int&, int const&>>));
		STATIC_ASSERT(!VEG_CONCEPT(move_assignable<veg::Tuple<int&, int const&>>));

		STATIC_ASSERT(!VEG_CONCEPT(copy_assignable<veg::Tuple<int&, float&>>));

		STATIC_ASSERT(!VEG_CONCEPT(copy_assignable<decltype(tup_ref)>));
		STATIC_ASSERT(!VEG_CONCEPT(move_assignable<decltype(tup_ref)>));
		STATIC_ASSERT(!VEG_CONCEPT(copyable<decltype(tup_ref)>));
		STATIC_ASSERT(VEG_CONCEPT(trivially_move_constructible<decltype(tup_ref)>));
		STATIC_ASSERT(!std::is_copy_constructible<decltype(tup_ref)>::value);
	}
	{
		using val_tup = veg::Tuple<int, bool>;
		val_tup tup_a = tuplify(5, true);
		val_tup tup_b = tuplify(3, false);

		unused(Tuple<long, bool>{
				val_tup{} | bind_back_once(
												tuple::map_i,
												tuplify(
														[](int i) -> long { return long(i); },
														[](bool b) -> bool { return b; })),
		});

		STATIC_ASSERT(val_tup{tuplify, 1, true} == val_tup{tuplify, 1, true});
		STATIC_ASSERT(
				cmp::cmp(
						ref(val_tup{tuplify, 1, true}), ref(val_tup{tuplify, 1, true})) ==
				cmp::Ordering::equal);

		CHECK(tup_a == (val_tup{tuplify, 5, true}));
		CHECK(tup_b == (val_tup{tuplify, 3, false}));
	}
	{
		using ref_tup = Tuple<RefMut<int>>;
		using val_tup = Tuple<int>;
		int i = 13;
		val_tup j{tuplify, 12};
		ref_tup a{tuplify, mut(i)};
		ref_tup b = j.as_mut();

		tuple::deref_assign(
				FWD(a),
				FWD(b) | bind_back_once(
										 tuple::map, [&](RefMut<int> x) VEG_ARROW(x.as_const())));
		CHECK(i == 12);
		CHECK(j[0_c] == 12);
	}

	CHECK(tup[0_c] == 1);
	CHECK(tup[1_c] == 'c');
	CHECK(tup[2_c]);

	{
		auto&& ref1 = FWD(tup)[2_c];
		auto&& ref2 = tup[2_c];
		CHECK(&ref1 != &tup[2_c]);
		CHECK(&ref2 == &tup[2_c]);
	}

	VEG_BIND(auto, (e, f, g), [&] { return clone(ref(tup)); }());
#if __cplusplus >= 201703L
	auto [i, c, b] = [&] { return clone(ref(tup)); }();
	CHECK(i == 1);
	CHECK(c == 'c');
	CHECK(b);
	veg::Tuple tup_deduce{tuplify, 1, 'c', true};
#endif

	STATIC_ASSERT(std::is_copy_assignable<veg::Tuple<int, char>>());
	STATIC_ASSERT(std::is_trivially_copyable<veg::Tuple<int, char>>());
	STATIC_ASSERT(!std::is_copy_assignable<veg::Tuple<int&, char&>>());
	STATIC_ASSERT(!std::is_copy_assignable<veg::Tuple<int&>>());
	ASSERT_SAME(decltype(tup[0_c]), int&);
	ASSERT_SAME(decltype(tup[0_c]), decltype(tup[0_c]));
	ASSERT_SAME(decltype(FWD(tup)[0_c]), decltype(FWD(tup)[0_c]));
	ASSERT_SAME(decltype(FWD(tup)[0_c]), int);

	ASSERT_SAME(
			decltype(tup.as_ref()), veg::Tuple<Ref<int>, Ref<char>, Ref<bool>>);
	ASSERT_SAME(
			decltype(tup.as_mut()),
			veg::Tuple<RefMut<int>, RefMut<char>, RefMut<bool>>);

	ASSERT_SAME(decltype(e), int&&);
	ASSERT_SAME(decltype((e)), int&);
	ASSERT_SAME(decltype(f), char&&);
	ASSERT_SAME(decltype((f)), char&);
	ASSERT_SAME(decltype((g)), bool&);

	ASSERT_SAME(decltype(tup_ref[0_c]), int&);

	ASSERT_SAME(decltype(tup_ref[1_c]), char&);
	ASSERT_SAME(decltype(tup_ref[2_c]), char&);
	ASSERT_SAME(decltype(FWD(tup_ref)[1_c]), char&&);
	ASSERT_SAME(decltype(FWD(tup_ref)[2_c]), char);

	ASSERT_SAME(decltype(tup_ref[3_c]), bool&);
	ASSERT_SAME(decltype(tup_ref[4_c]), bool const&);
	ASSERT_SAME(decltype(FWD(tup_ref)[3_c]), bool&);
	ASSERT_SAME(decltype(FWD(tup_ref)[4_c]), bool const&);

#if __cplusplus >= 201703L
	ASSERT_SAME(decltype(i), int);
	ASSERT_SAME(decltype((i)), int&);
	ASSERT_SAME(decltype((b)), bool&);
	ASSERT_SAME(decltype(tup_deduce), veg::Tuple<int, char, bool>);
#endif

	STATIC_ASSERT(Tuple<int>{tuplify, 1} == Tuple<int>{tuplify, 1});
	STATIC_ASSERT(Tuple<int>{tuplify, 1} == Tuple<int>{tuplify, 1});
	STATIC_ASSERT(Tuple<int>{tuplify, 1} != Tuple<int>{tuplify, 2});
	STATIC_ASSERT(
			Tuple<int, double>{tuplify, 1, 2.0} == Tuple<int, double>{tuplify, 1, 2});
	STATIC_ASSERT(
			Tuple<int, double>{tuplify, 1, 2.0} == Tuple<int, double>{tuplify, 1, 2});
	STATIC_ASSERT(
			Tuple<int, double>{tuplify, 1, 2.0} != Tuple<int, double>{tuplify, 2, 2});
}

TEST_CASE("tuple: nested") {
	using namespace veg;
	Tuple<int, Tuple<int, float>> tup{tuplify, 1, {tuplify, 2, 3.0F}};
	CHECK(tup[0_c] == 1);

	CHECK(tup[1_c][1_c] == 3.0F);
	CHECK(tup[1_c][0_c] == 2);
	ASSERT_SAME(decltype(tup[1_c][0_c]), int&);
	ASSERT_SAME(decltype(tup[1_c][0_c]), int&);
	ASSERT_SAME(decltype(FWD(tup)[1_c][0_c]), int);

	STATIC_ASSERT(sizeof(Tuple<int>) == sizeof(int));
	STATIC_ASSERT(sizeof(Tuple<Tuple<int>>) == sizeof(int));
	STATIC_ASSERT(sizeof(Tuple<Tuple<Tuple<int>>>) == sizeof(int));
}

TEST_CASE("tuple: cmp") {
	using namespace veg;
	using veg::cmp::cmp;
	constexpr double nan = std::numeric_limits<double>::quiet_NaN();

	{
		constexpr auto t1 = tuplify(1, 2, 3);
		constexpr auto t2 = tuplify(1, 2, 3);
		constexpr auto t3 = tuplify(0, 2, 3);
		constexpr auto t4 = tuplify(1, 2, 4);

		STATIC_ASSERT(cmp(ref(t1), ref(t2)) == cmp::Ordering::equal);
		STATIC_ASSERT(cmp(ref(t1), ref(t3)) == cmp::Ordering::greater);
		STATIC_ASSERT(cmp(ref(t1), ref(t4)) == cmp::Ordering::less);
		STATIC_ASSERT(cmp(ref(t2), ref(t3)) == cmp::Ordering::greater);
		STATIC_ASSERT(cmp(ref(t2), ref(t4)) == cmp::Ordering::less);
		STATIC_ASSERT(cmp(ref(t3), ref(t4)) == cmp::Ordering::less);
	}
	CHECK(
			cmp( //
					ref(tuplify(1, 1, nan)),
					ref(tuplify(1, 1, 0))) == cmp::Ordering::unordered);

	CHECK(
			cmp( //
					ref(tuplify(0, 1, nan)),
					ref(tuplify(1, 1, 0))) == cmp::Ordering::less);

	CHECK(
			cmp( //
					ref(tuplify(1, 2, nan)),
					ref(tuplify(1, 1, 0))) == cmp::Ordering::greater);
}

TEST_CASE("tuple: empty") {
	using namespace veg;
	Tuple<> t1{};
	Tuple<> t2;
	CHECK(t1 == t2);
	STATIC_ASSERT(sizeof(t1) == 1);
	STATIC_ASSERT(Tuple<>{} == Tuple<>{});
}

TEST_CASE("tuple: tuplify") {
	using namespace veg;
	Tuple<int, double> t1{tuplify, 1, 1.5};
	Tuple<long, double> t2(tuplify, 3, 2.5);
	Tuple<long, double> t3 =
			FWD(t1) | bind_back_once(
										tuple::map_i,
										tuplify(
												[](int x) -> long { return long(x); },
												[](double x) -> double { return x; }));

	Tuple<int, double> t4 =
			FWD(t3) | bind_back_once(
										tuple::map_i,
										tuplify(
												[](long x) -> int { return int(x); },
												[](double x) -> double { return x; }));

	STATIC_ASSERT(sizeof(t2) == sizeof(long) + sizeof(double));

	CHECK(t1 == t4);
	tuple::deref_assign(t2.as_mut(), t1.as_ref());

	CHECK(
			t1 ==
			(veg::clone(ref(t2)) | bind_back_once(
																 tuple::map_i,
																 tuplify(
																		 [](long i) -> int { return int(i); },
																		 [](double d) -> double { return d; }))));
}

TEST_CASE("tuple: get") {
	using namespace veg;
	i64 arr[] = {1, 2, 3};
	nb::get<0>{}(arr);
	nb::get<0>{}(FWD(arr));
}

TEST_CASE("tuple: ✨ functional programming ✨") {
	using namespace veg;
	using veg::clone;

	{
		constexpr auto x = tuplify(i64(1), i64(2), i64(3));
		CHECK((x.as_ref() | bind_back_once(tuple::map, [](Ref<i64> i) -> float {
						 return 2 * float(*i);
					 })) == tuplify(2.0F, 4.0F, 6.0F));
		i64 acc = 0;
		x.as_ref() |
				bind_back_once(tuple::for_each, [&](Ref<i64> i) -> void { acc += *i; });
		CHECK(acc == 1 + 2 + 3);
	}

	{
		constexpr auto x = tuplify(2.5, 3, tuplify(&tuplify, 10.0));
		tuple::unpack(
				clone(ref(x)),
				[](double d, int i, Tuple<decltype(&tuplify), double> tup) {
					CHECK(d == 2.5);
					CHECK(i == i);
					CHECK(tup == tuplify(&tuplify, 10.0));
					CHECK(tup[0_c] == &tuplify);
					CHECK(tup[1_c] == 10.0);
				});
	}
	{
		CHECK(
				Tuple<Tuple<int>>{
						tuplify(tuplify(2.3)) |
								bind_back_once(
										tuple::map,
										[](Tuple<double> inner) -> Tuple<int> {
											return FWD(inner) |
			                       bind_back_once(
																 tuple::map, fn::copy_fn([](double x) -> int {
																	 return int(x);
																 }));
										}),
				}[0_c][0_c] == 2);
	}

	{
		STATIC_ASSERT(
				tuple::cat(tuplify(1.0, 2), tuplify(1, 4.0)) ==
				tuplify(1.0, 2, 1, 4.0));
	}

	{
		STATIC_ASSERT(tuplify(1.0, 2) == tuplify(1.0, 2));
		STATIC_ASSERT(
				tuple::zip(tuplify(1.0, 2)) == tuplify(tuplify(1.0), tuplify(2)));
		STATIC_ASSERT(
				tuple::zip(tuplify(1.0), tuplify(2)) == tuplify(tuplify(1.0, 2)));

		constexpr void* p = nullptr;
		STATIC_ASSERT(
				tuple::zip(tuplify(p, 'c'), tuplify(1.0, 2), tuplify(13, 4.0)) ==
				tuplify(tuplify(p, 1.0, 13), tuplify('c', 2, 4.0)));
	}

	{ STATIC_ASSERT(tuple::zip(tuplify(), tuplify(), tuplify()) == tuplify()); }

	{
		constexpr auto x = tuplify(i64(1), float(2.5), i64(3));
		float acc = 0;

		x.as_ref() | bind_back_once(
										 tuple::for_each_i,
										 tuplify(
												 [&](Ref<i64> i) -> void { acc += 0 * float(*i); },
												 [&](Ref<float> i) -> void { acc += 1 * float(*i); },
												 [&](Ref<i64> i) -> void { acc += 2 * float(*i); }));
		CHECK(acc == 1 * 0 + 2.5F * 1 + 3 * 2);
	}
}
#include "veg/internal/epilogue.hpp"
