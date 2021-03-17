// TODO: proper test?

// the first time the header is included, it must be done in the global
// namespace
#include "veg/assert.hpp"
#include <fmt/core.h>
#include <iostream>

namespace {
namespace A {
void print(char const* s, int n) {
	fmt::print("{} {}\n", s, n);
}

template <typename T, std::size_t N>
void my_fn(std::array<T, N> arr) {
	// prints an error message if the condition fails then continues execution
	VEG_EXPECT(arr[0] == 0);

	// prints an error message if the condition fails then continues execution
	VEG_DEBUG_EXPECT(arr[0] == 3);

#define NDEBUG
	// subsequent inclusions are valid in any context
#include "veg/assert.hpp"
	VEG_DEBUG_EXPECT(arr[0] == 3);

	// prints a warning (first argument) to stderr alongside the error message if
	// the condition fails
	// the string is never constructed if the condition holds
	VEG_EXPECT_ELSE(fmt::format("sizeof(T) is not {}", N), sizeof(T) == N);

#undef NDEBUG
	// subsequent inclusions are valid in any context
#include "veg/assert.hpp"

	VEG_DEBUG_EXPECT_ALL_OF((arr[0] == 0), (arr[2] == 1), (arr[1] == 2));

	VEG_DEBUG_EXPECT_ALL_OF_ELSE(
			("assertion 0", arr[0] == 0), // print message "assertion 0"
			("", arr[2] == 1),
			("", arr[1] == 2));

	// VEG{,_DEBUG}_ASSERT{,_ELSE,_ALL_OF_ELSE} work the same, but terminate
	// after printing the error
}
} // namespace A
} // namespace

struct S {
	friend auto
	tag_invoke(veg::tag_t<veg::assert::fn::to_string_fn> /*tag*/, S /*s*/)
			-> std::string {
		VEG_ASSERT(1 == 0);
		return "";
	}
	explicit operator bool() const { return false; }
};

auto main() -> int {
	A::my_fn(std::array<int, 3>{{1, 2, 3}});

	// should terminate because of recursive failed assertion
	VEG_ASSERT(S{});
}
