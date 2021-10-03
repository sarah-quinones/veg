#include "static_assert.hpp"
#include <limits>
#include <veg/tuple.hpp>
#include <utility>
#include <doctest.h>
#include "veg/memory/placement.hpp"
#include "veg/functional/overload.hpp"
#include "veg/functional/indexed.hpp"
#include "veg/functional/copy_fn.hpp"
#include "veg/internal/prologue.hpp"

#define FWD VEG_FWD
#define ASSERT_SAME(...) STATIC_ASSERT(::std::is_same<__VA_ARGS__>::value)

using namespace veg::literals;
using namespace veg::tags;

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

	auto tup = tuple::make(1, 'c', true);
	veg::Tuple<int, char&&, char, bool&, bool const&> tup_ref{
			direct,
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
		using val_tup = veg::Tuple<int, bool>;

		STATIC_ASSERT(VEG_CONCEPT(swappable<val_tup&, val_tup&>));
		STATIC_ASSERT(!VEG_CONCEPT(swappable<val_tup&&, val_tup&&>));
		STATIC_ASSERT(!VEG_CONCEPT(swappable<val_tup&, val_tup&&>));

		STATIC_ASSERT(VEG_CONCEPT(nothrow_swappable<val_tup&, val_tup&>));
		STATIC_ASSERT(!VEG_CONCEPT(nothrow_swappable<val_tup&&, val_tup&&>));
		STATIC_ASSERT(!VEG_CONCEPT(nothrow_swappable<val_tup&, val_tup&&>));
	}
	{
		using val_tup = veg::Tuple<int, bool>;
		val_tup tup_a = tuple::make(5, true);
		val_tup tup_b = tuple::make(3, false);

		unused(Tuple<long, bool>{
				val_tup{}.map_i(fn::overload(
						[](Fix<0> /*tag*/, int i) -> long { return long(i); },
						[](Fix<1> /*tag*/, bool b) -> bool { return b; })),
		});

		veg::swap(tup_a, tup_b);
		CHECK(tup_a[0_c] == 3);
		CHECK(tup_b[0_c] == 5);
		CHECK(tup_a[1_c] == false);
		CHECK(tup_b[1_c] == true);
		STATIC_ASSERT(val_tup{direct, 1, true} == val_tup{direct, 1, true});
		STATIC_ASSERT(
				noexcept(val_tup{direct, 1, true} == val_tup{direct, 1, true}));
		STATIC_ASSERT(
				cmp::cmp(
						ref(val_tup{direct, 1, true}), ref(val_tup{direct, 1, true})) ==
				cmp::Ordering::equal);

		CHECK(tup_a == (val_tup{direct, 3, false}));
		CHECK(tup_b == (val_tup{direct, 5, true}));
	}
	{
		using ref_tup = Tuple<RefMut<int>>;
		using val_tup = Tuple<int>;
		int i = 13;
		val_tup j{direct, 12};
		ref_tup a{direct, {as_mut, i}};
		ref_tup b = j.as_mut();

		tuple::deref_swap(FWD(a), FWD(b));

		CHECK(i == 12);
		CHECK(j[0_c] == 13);

		tuple::deref_assign(FWD(a), FWD(b).as_const());
		CHECK(i == 13);
		CHECK(j[0_c] == 13);
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

	VEG_BIND(auto, (e, f, g), [&] { return clone(tup); }());
#if __cplusplus >= 201703L
	auto [i, c, b] = [&] { return clone(tup); }();
	CHECK(i == 1);
	CHECK(c == 'c');
	CHECK(b);
	veg::Tuple tup_deduce{direct, 1, 'c', true};
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

	STATIC_ASSERT(Tuple<int>{direct, 1} == Tuple<int>{direct, 1});
	STATIC_ASSERT(Tuple<int>{direct, 1} == Tuple<int>{direct, 1});
	STATIC_ASSERT(Tuple<int>{direct, 1} != Tuple<int>{direct, 2});
	STATIC_ASSERT(
			Tuple<int, double>{direct, 1, 2.0} == Tuple<int, double>{direct, 1, 2});
	STATIC_ASSERT(
			Tuple<int, double>{direct, 1, 2.0} == Tuple<int, double>{direct, 1, 2});
	STATIC_ASSERT(
			Tuple<int, double>{direct, 1, 2.0} != Tuple<int, double>{direct, 2, 2});
}

TEST_CASE("tuple: nested") {
	using namespace veg;
	Tuple<int, Tuple<int, float>> tup{direct, 1, {direct, 2, 3.0F}};
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
		constexpr auto t1 = tuple::make(1, 2, 3);
		constexpr auto t2 = tuple::make(1, 2, 3);
		constexpr auto t3 = tuple::make(0, 2, 3);
		constexpr auto t4 = tuple::make(1, 2, 4);

		STATIC_ASSERT(cmp(ref(t1), ref(t2)) == cmp::Ordering::equal);
		STATIC_ASSERT(cmp(ref(t1), ref(t3)) == cmp::Ordering::greater);
		STATIC_ASSERT(cmp(ref(t1), ref(t4)) == cmp::Ordering::less);
		STATIC_ASSERT(cmp(ref(t2), ref(t3)) == cmp::Ordering::greater);
		STATIC_ASSERT(cmp(ref(t2), ref(t4)) == cmp::Ordering::less);
		STATIC_ASSERT(cmp(ref(t3), ref(t4)) == cmp::Ordering::less);
	}
	CHECK(
			cmp( //
					ref(tuple::make(1, 1, nan)),
					ref(tuple::make(1, 1, 0))) == cmp::Ordering::unordered);

	CHECK(
			cmp( //
					ref(tuple::make(0, 1, nan)),
					ref(tuple::make(1, 1, 0))) == cmp::Ordering::less);

	CHECK(
			cmp( //
					ref(tuple::make(1, 2, nan)),
					ref(tuple::make(1, 1, 0))) == cmp::Ordering::greater);
}

TEST_CASE("tuple: empty") {
	using namespace veg;
	Tuple<> t1{};
	Tuple<> t2;
	CHECK(t1 == t2);
	STATIC_ASSERT(sizeof(t1) == 1);
	STATIC_ASSERT(Tuple<>{} == Tuple<>{});
}

TEST_CASE("tuple: direct") {
	using namespace veg;
	Tuple<int, double> t1{direct, 1, 1.5};
	Tuple<long, double> t2(direct, 3, 2.5);
	Tuple<long, double> t3 = FWD(t1).map_i(fn::overload(
			fn::nb::indexed<0>{}([](int x) -> long { return long(x); }),
			fn::nb::indexed<1>{}([](double x) -> double { return x; })));
	Tuple<int, double> t4 = FWD(t3).map_i(fn::copy_fn(fn::overload(
			fn::nb::indexed<0>{}([](long x) -> int { return int(x); }),
			fn::nb::indexed<1>{}([](double x) -> double { return x; }))));

	STATIC_ASSERT(sizeof(t2) == sizeof(long) + sizeof(double));

	CHECK(t1 == t4);
	tuple::deref_assign(t2.as_mut(), t1.as_ref());

	CHECK(
			t1 == veg::clone(t2).map_i(fn::overload(
								[](Fix<0>, long i) -> int { return int(i); },
								[](Fix<1>, double d) -> double { return d; })));
}

TEST_CASE("tuple: get") {
	using namespace veg;
	i64 arr[] = {1, 2, 3};
	nb::get<0>{}(arr);
	nb::get<0>{}(FWD(arr));
}

TEST_CASE("tuple: *sparkles* functional programming *sparkles*") {
	using namespace veg;
	using veg::clone;

	{
		constexpr auto x = tuple::make(i64(1), i64(2), i64(3));
		CHECK(x.as_ref().map([](Ref<i64> i) -> float {
			return 2 * float(*i);
		}) == tuple::make(2.0F, 4.0F, 6.0F));
		i64 acc = 0;
		x.as_ref().for_each([&](Ref<i64> i) -> void { acc += *i; });
		CHECK(acc == 1 + 2 + 3);
	}

	{
		constexpr auto x = tuple::make(2.5, 3, tuple::make(&tuple::make, 10.0));
		tuple::unpack(
				clone(x),
				[](double d, int i, Tuple<decltype(&tuple::make), double> tup) {
					CHECK(d == 2.5);
					CHECK(i == i);
					CHECK(tup == tuple::make(&tuple::make, 10.0));
					CHECK(tup[0_c] == &tuple::make);
					CHECK(tup[1_c] == 10.0);
				});
	}
	{
		CHECK(
				Tuple<Tuple<int>>{
						tuple::make(tuple::make(2.3))
								.map([](Tuple<double> inner) -> Tuple<int> {
									return FWD(inner).map(
											fn::copy_fn([](double x) -> int { return int(x); }));
								}),
				}[0_c][0_c] == 2);
	}

	{
		STATIC_ASSERT(
				tuple::cat(tuple::make(1.0, 2), tuple::make(1, 4.0)) ==
				tuple::make(1.0, 2, 1, 4.0));
	}

	// sfinae test
	STATIC_ASSERT(!VEG_CONCEPT(invocable<struct tuple::nb::zip, int>));
	STATIC_ASSERT(!VEG_CONCEPT(
			invocable<struct tuple::nb::zip, Tuple<int>, Tuple<float, bool>>));
	{
		STATIC_ASSERT(tuple::make(1.0, 2) == tuple::make(1.0, 2));
		STATIC_ASSERT(
				tuple::zip(tuple::make(1.0, 2)) ==
				tuple::make(tuple::make(1.0), tuple::make(2)));
		STATIC_ASSERT(
				tuple::zip(tuple::make(1.0), tuple::make(2)) ==
				tuple::make(tuple::make(1.0, 2)));

		constexpr void* p = nullptr;
		STATIC_ASSERT(
				tuple::zip(
						tuple::make(p, 'c'), tuple::make(1.0, 2), tuple::make(13, 4.0)) ==
				tuple::make(tuple::make(p, 1.0, 13), tuple::make('c', 2, 4.0)));
	}

	{
		STATIC_ASSERT(
				tuple::zip(tuple::make(), tuple::make(), tuple::make()) ==
				tuple::make());
	}

#if __cplusplus >= 201402L
	{
		constexpr auto x = tuple::make(i64(1), float(2.5), i64(3));
		float acc = 0;

		x.as_ref().for_each_i(
				[&](auto N, auto i) -> void { acc += float(*i) * i64{N}; });
		CHECK(acc == 1 * 0 + 2.5F * 1 + 3 * 2);
	}
#endif
}
#include "veg/internal/epilogue.hpp"
