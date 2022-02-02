#include "veg/util/assert.hpp"
#include "veg/util/timer.hpp"
#include "veg/internal/parse.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <exception>
#include <string>
#include <new>
#include "veg/internal/prologue.hpp"

#ifdef __unix__
#include <unistd.h>

#define ISATTY ::isatty
#define FILENO ::fileno
#else

#ifndef __APPLE__
#include <io.h>
#define ISATTY ::_isatty
#define FILENO ::_fileno
#else
#include <sys/uio.h>
#include <unistd.h>
#define ISATTY ::isatty
#define FILENO ::fileno
#endif

#endif

namespace veg {
namespace _detail {
namespace type_parse {
void function_decl_to_str(RefMut<std::string> str, FunctionDecl decl) noexcept;
} // namespace type_parse
[[noreturn]] void terminate() VEG_ALWAYS_NOEXCEPT {
	std::terminate();
}
} // namespace _detail

namespace _detail {
String::~String() {
	std::free(self.ptr);
}

void String::eprintln(std::FILE* f) const VEG_ALWAYS_NOEXCEPT {
	std::fwrite(self.ptr, 1, self.len, f);
	std::fputc('\n', f);
}
void String::reserve(usize new_cap) {
	if (new_cap > self.cap) {
		auto* new_alloc = static_cast<char*>(std::realloc(self.ptr, new_cap));
		if (new_alloc == nullptr) {
			std::terminate();
		}
		self.ptr = new_alloc;
		self.cap = new_cap;
	}
}
void String::resize(usize new_len) {
	reserve(new_len);
	self.len = new_len;
}
void String::insert(usize pos, char const* data_, usize len) {
	usize old_size = size();
	usize pre_new_size = len + old_size;
	usize new_size =
			(pre_new_size > (2 * old_size)) ? pre_new_size : (2 * old_size);
	reserve(new_size);
	resize(pre_new_size);
	if (len > 0) {
		std::memmove(data() + pos + len, data() + pos, std::size_t(old_size - pos));
		std::memmove(data() + pos, data_, std::size_t(len));
	}
}
void String::insert_newline(usize pos) {
	insert(pos, mem::addressof('\n'), 1);
	++pos;
	for (usize i = 0; i < indent_level; ++i) {
		insert(pos, mem::addressof('\t'), 1);
		++pos;
	}
}

thread_local i64
		counter = // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
		0;
void incr_counter() VEG_ALWAYS_NOEXCEPT {
	if (counter > 0) {
		std::fputs(
				"assertion failed during the handling of another failed assertion\n",
				stderr);
		terminate();
	}
	++counter;
}
void decr_counter() VEG_ALWAYS_NOEXCEPT {
	--counter;
}

auto ByteStringView::starts_with(ByteStringView other) const VEG_ALWAYS_NOEXCEPT
		-> bool {
	return size() >= other.size() &&
	       std::memcmp(data(), other.data(), other.size()) == 0;
}
auto ByteStringView::operator==(ByteStringView other) const VEG_ALWAYS_NOEXCEPT
		-> bool {
	return size() == other.size() &&
	       std::memcmp(data(), other.data(), other.size()) == 0;
}
ByteStringView::ByteStringView(char const* str) VEG_ALWAYS_NOEXCEPT
		: ByteStringView{str, std::strlen(str)} {}

struct color_t {
	std::uint8_t r;
	std::uint8_t g;
	std::uint8_t b;
};

constexpr color_t default_color = {0, 0, 0};
constexpr color_t olive = {128, 128, 0};
constexpr color_t orange_red = {255, 69, 0};
constexpr color_t azure = {240, 255, 255};
constexpr color_t gray = {128, 128, 128};

auto with_color(color_t c, std::string s) -> std::string {
	if (!ISATTY(FILENO(stderr))) {
		return s;
	}
	s = "\x1b[38;2;000;000;000m" + VEG_FWD(s) + "\x1b[0m";

	size_t i = 7;
	s[i++] = char('0' + c.r / 100);
	s[i++] = char('0' + (c.r % 100) / 10);
	s[i++] = char('0' + (c.r % 10));
	++i;

	s[i++] = char('0' + c.g / 100);
	s[i++] = char('0' + (c.g % 100) / 10);
	s[i++] = char('0' + (c.g % 10));
	++i;

	s[i++] = char('0' + c.b / 100);
	s[i++] = char('0' + (c.b % 100) / 10);
	s[i++] = char('0' + (c.b % 10));

	return s;
}

struct assertion_data {
	bool finished_setup;
	ByteStringView expr;
	ByteStringView callback;
	ByteStringView op;
	std::string lhs;
	std::string rhs;
};

thread_local Vec<assertion_data>
		failed_asserts // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
		= {};

cleanup::~cleanup() VEG_ALWAYS_NOEXCEPT {
	failed_asserts.clear();
}

using std::size_t;

auto to_owned(ByteStringView ref) -> std::string {
	return {ref.data(), static_cast<std::size_t>(ref.size())};
}

auto find(ByteStringView sv, char c) -> char const* {
	char const* it = sv.data();
	for (; it < sv.data() + sv.size(); ++it) {
		if (*it == c) {
			break;
		}
	}
	return it;
}

auto on_fail(long line, ByteStringView file, ByteStringView func, bool is_fatal)
		-> std::string {
	auto _clear = [&] { failed_asserts.clear(); };
	auto&& clear = defer(_clear);
	(void)clear;

	std::string output;

	output += '\n';
	output += with_color(
			olive,
			"========================================"
			"=======================================");
	output += '\n';
	output += with_color(orange_red, std::to_string(failed_asserts.len()));
	output += " assertion";
	if (failed_asserts.len() > 1) {
		output += 's';
	}
	output += ' ';
	output += "failed";
	output += '\n';
	if (func.size() > 0) {
		output += "in function:\n";
		output += to_owned(func);
		output += '\n';
		output +=
				with_color(gray, to_owned(file) + (':' + std::to_string(line)) + ':');
		output += '\n';
		veg::_detail::type_parse::function_decl_to_str(
				mut(output),
				veg::_detail::type_parse::parse_function_decl({
						from_raw_parts,
						{func.data(), func.size()},
				}));
		output += '\n';
	}

	char const* separator = "";

	for (isize i = 0; i < failed_asserts.len(); ++i) {
		auto& a = failed_asserts[i];
		ByteStringView msg{a.callback};
		char const* newline = find(msg, '\n');
		bool multiline = newline != nullptr;

		output += separator;

		if (is_fatal) {
			output += with_color(orange_red, "fatal ");
		}
		output += "assertion ";
		output += '`';
		output += with_color(orange_red, to_owned(a.expr));
		output += '\'';
		output += " failed:";

		if (!multiline) {
			output += ' ';
			output += to_owned(msg);
		} else {
			char const* b = msg.data();
			char const* e = newline;
			char const* end = msg.data() + msg.size();

			while (b != end) {

				output += "\n > ";
				output += {b, e};

				if (e == end) {
					b = end;
				} else {
					b = e + 1;
					e = find({b, usize((msg.data() + msg.size()) - b)}, '\n');
				}
			}
		}

		output += "\nassertion expands to: `";
		output += with_color(orange_red, a.lhs + to_owned(a.op) + a.rhs);
		output += '\'';
		output += '\n';
		separator = "\n";
	}

	output += with_color(
			olive,
			"========================================"
			"=======================================");
	output += '\n';

	return output;
}

void on_expect_fail(long line, ByteStringView file, ByteStringView func) {
	auto str = on_fail(line, file, func, false);
	std::fwrite(str.data(), 1, str.size(), stderr);
}

[[noreturn]] void
on_assert_fail(long line, ByteStringView file, ByteStringView func) {
	auto str = on_fail(line, file, func, true);
	std::fwrite(str.data(), 1, str.size(), stderr);
	std::terminate();
}

void set_assert_params1( //
		ByteStringView op,   //
		String lhs,          //
		String rhs           //
) {
	bool success = false;

	auto _clear = [&] {
		if (!success) {
			failed_asserts.clear();
		}
	};

	auto&& clear = defer(_clear);
	(void)clear;

	failed_asserts.push({
			false,
			ByteStringView{"", 0},
			empty_str,
			op,
			std::string{lhs.data(), static_cast<std::size_t>(lhs.size())},
			std::string{rhs.data(), static_cast<std::size_t>(rhs.size())},
	});

	success = true;
}
void set_assert_params2(       //
		ByteStringView expression, //
		ByteStringView msg         //
		) VEG_ALWAYS_NOEXCEPT {
	auto& a = failed_asserts[failed_asserts.len() - 1];
	a.finished_setup = true;
	a.expr = expression;
	a.callback = msg;
}

auto snprintf1(char* out, usize n, unsigned type, void* arg) -> usize {
	unsigned type_id = type % 4U;

	switch (type_id) {
	case 0: // signed
		return usize(
				std::snprintf(out, n, "%lld", *static_cast<long long signed*>(arg)));
	case 1: // unsigned
		return usize(
				std::snprintf(out, n, "%llu", *static_cast<long long unsigned*>(arg)));
	case 2: // pointer
		return usize(std::snprintf(out, n, "%p", arg));
	case 3: { // float
		return usize(
				std::snprintf(out, n, "%+.4Le", *static_cast<long double*>(arg)));
	}
	default:
		terminate();
	}
}
} // namespace _detail
} // namespace veg
#include "veg/internal/epilogue.hpp"
