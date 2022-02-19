#include "veg/util/assert.hpp"
#include "veg/util/timer.hpp"
#include "veg/internal/parse.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <exception>
#include <new>
#include "veg/internal/prologue.hpp"

#ifdef __APPLE__
#include <sys/uio.h>
#endif

#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#define HAS_COLOR_CHECK 1
#else
#define HAS_COLOR_CHECK 0
#endif

namespace veg {
namespace _detail {
void write_utf8_to(std::FILE* f, char const* ptr, usize len) noexcept {
	// TODO: handle utf8 on windows
	std::fwrite(ptr, 1, len, f);
}

namespace type_parse {
void function_decl_to_str(RefMut<Vec<char>> str, FunctionDecl decl) noexcept;
} // namespace type_parse
[[noreturn]] void terminate() VEG_ALWAYS_NOEXCEPT {
	std::terminate();
}
} // namespace _detail

namespace _detail {
String::~String() {
	std::free(self.ptr);
}

void String::fprintln(std::FILE* f) const VEG_ALWAYS_NOEXCEPT {
	_detail::write_utf8_to(f, self.ptr, self.len);
	std::fputc('\n', f);
}
void String::reserve(usize new_cap) noexcept {
	if (new_cap > self.cap) {
		auto* new_alloc = static_cast<char*>(std::realloc(self.ptr, new_cap));
		if (new_alloc == nullptr) {
			std::terminate();
		}
		self.ptr = new_alloc;
		self.cap = new_cap;
	}
}
void String::resize(usize new_len) noexcept {
	reserve(new_len);
	self.len = new_len;
}
void String::insert(usize pos, char const* data_, usize len) noexcept {
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

void String::insert_newline(usize pos) noexcept {
	append_literal(u8"\n");
	++pos;
	for (usize i = 0; i < indent_level; ++i) {
		append_literal(u8"\t");
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

struct Color {
	std::uint8_t r;
	std::uint8_t g;
	std::uint8_t b;
};

constexpr Color default_color = {0, 0, 0};
constexpr Color olive = {128, 128, 0};
constexpr Color orange_red = {255, 69, 0};
constexpr Color azure = {240, 255, 255};
constexpr Color gray = {128, 128, 128};

auto with_color(Color c, Vec<char> s) -> Vec<char> {
#if defined(HAS_COLOR_CHECK)

	if (::isatty(::fileno(stderr)) == 0) {
		return s;
	}
	auto& prefix = u8"\x1b[38;2;000;000;000m";
	auto& suffix = u8"\x1b[0m";

	auto old_len = s.len();
	s.resize_for_overwrite(
			old_len + isize{sizeof(prefix)} + isize{sizeof(suffix)});

	std::memmove(s.ptr_mut() + sizeof(prefix), s.ptr(), old_len);
	std::memcpy(s.ptr_mut(), prefix, sizeof(prefix));
	std::memcpy(
			s.ptr_mut() + (isize{sizeof(prefix)} + old_len), suffix, sizeof(suffix));

	isize i = 7;

	auto zero = char(u8"0"[0]);

	s[i++] = char(zero + c.r / 100);
	s[i++] = char(zero + (c.r % 100) / 10);
	s[i++] = char(zero + (c.r % 10));
	++i;

	s[i++] = char(zero + c.g / 100);
	s[i++] = char(zero + (c.g % 100) / 10);
	s[i++] = char(zero + (c.g % 10));
	++i;

	s[i++] = char(zero + c.b / 100);
	s[i++] = char(zero + (c.b % 100) / 10);
	s[i++] = char(zero + (c.b % 10));

#endif
	return s;
}

struct assertion_data {
	bool finished_setup;
	ByteStringView expr;
	ByteStringView callback;
	ByteStringView op;
	Vec<char> lhs;
	Vec<char> rhs;
};

thread_local Vec<assertion_data>
		failed_asserts // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
		= {};

cleanup::~cleanup() VEG_ALWAYS_NOEXCEPT {
	failed_asserts.clear();
}

using std::size_t;

auto to_owned(ByteStringView ref) -> Vec<char> {
	Vec<char> rv;
	rv.resize_for_overwrite(isize(ref.size()));
	std::memcpy(rv.ptr_mut(), ref.data(), ref.size());
	;
	return rv;
}
template <usize N>
auto to_owned(char const (&ref)[N]) -> Vec<char> {
	return to_owned(ByteStringView{ref, N - 1});
}
template <usize N>
auto to_slice(char const (&ref)[N]) -> Slice<char> {
	return {unsafe, from_raw_parts, ref, N};
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

void append(RefMut<Vec<char>> a, Slice<char> b) {
	auto old_len = a->len();
	a->resize_for_overwrite(old_len + b.len());
	std::memcpy(a->ptr_mut() + old_len, b.ptr(), b.len());
}
void append_num(RefMut<Vec<char>> a, long long arg) {
	auto len = isize(std::snprintf(nullptr, 0, "%lld", arg));

	auto old_len = a->len();
	a->resize_for_overwrite(old_len + len + 1);
	std::snprintf(a->ptr_mut() + old_len, len + 1, "%lld", arg);
	a->resize_for_overwrite(old_len + len);
}

auto on_fail(long line, ByteStringView file, ByteStringView func, bool is_fatal)
		-> Vec<char> {
	auto _clear = [&] { failed_asserts.clear(); };
	auto&& clear = defer(_clear);
	(void)clear;

	Vec<char> output;
	auto ln = to_slice("\n");

	append(mut(output), ln);
	auto print_separator = [&]() noexcept -> void {
		append(
				mut(output),
				with_color(
						olive,
						to_owned("========================================"
		                 "======================================="))
						.as_ref());
	};
	print_separator();

	append(mut(output), ln);
	append_num(mut(output), failed_asserts.len());
	append(mut(output), to_slice(" assertion"));

	if (failed_asserts.len() > 1) {
		append(mut(output), to_slice("s"));
	}

	append(mut(output), to_slice(" failed\n"));

	if (func.size() > 0) {
		append(mut(output), to_slice("in function:\n"));
		append(
				mut(output), {unsafe, from_raw_parts, func.data(), isize(func.size())});

		append(mut(output), ln);

		{
			auto tmp = to_owned(file);
			append(mut(tmp), to_slice(":"));
			append_num(mut(tmp), line);
			append(mut(tmp), to_slice(":"));
			append(mut(output), with_color(gray, VEG_FWD(tmp)).as_ref());
		}

		append(mut(output), ln);

		veg::_detail::type_parse::function_decl_to_str(
				mut(output),
				veg::_detail::type_parse::parse_function_decl({
						from_raw_parts,
						{func.data(), func.size()},
				}));
		append(mut(output), ln);
	}

	auto separator = to_slice("");

	for (isize i = 0; i < failed_asserts.len(); ++i) {
		auto& a = failed_asserts[i];
		ByteStringView msg{a.callback};
		char const* newline = find(msg, '\n');
		bool multiline = newline != nullptr;

		append(mut(output), separator);

		if (is_fatal) {
			append(mut(output), with_color(orange_red, to_owned("fatal ")).as_ref());
		}
		append(mut(output), to_slice("assertion `"));

		append(mut(output), with_color(orange_red, to_owned(a.expr)).as_ref());
		append(mut(output), to_slice("' failed:"));

		if (!multiline) {
			append(mut(output), to_slice(" "));
			append(mut(output), to_owned(msg).as_ref());
		} else {
			char const* b = msg.data();
			char const* e = newline;
			char const* end = msg.data() + msg.size();

			while (b != end) {

				append(mut(output), to_slice("\n > "));
				append(mut(output), {unsafe, from_raw_parts, b, e - b});

				if (e == end) {
					b = end;
				} else {
					b = e + 1;
					e = find({b, usize((msg.data() + msg.size()) - b)}, '\n');
				}
			}
		}

		append(mut(output), to_slice("\nassertion expands to: `"));
		{
			Vec<char> tmp{a.lhs};
			append(mut(tmp), to_owned(a.op).as_ref());
			append(mut(tmp), a.rhs.as_ref());
			append(mut(output), with_color(orange_red, VEG_FWD(tmp)).as_ref());
		}
		append(mut(output), to_slice("'\n"));
		separator = to_slice("\n");
	}

	print_separator();
	append(mut(output), ln);

	return output;
}

void on_expect_fail(long line, ByteStringView file, ByteStringView func) {
	auto str = on_fail(line, file, func, false);
	_detail::write_utf8_to(stderr, str.ptr(), str.len());
}

[[noreturn]] void
on_assert_fail(long line, ByteStringView file, ByteStringView func) {
	auto str = on_fail(line, file, func, true);
	_detail::write_utf8_to(stderr, str.ptr(), str.len());
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

	auto lhs_vec = Vec<char>{};
	auto rhs_vec = Vec<char>{};

	lhs_vec.resize_for_overwrite(isize(lhs.size()));
	rhs_vec.resize_for_overwrite(isize(rhs.size()));
	std::memcpy(lhs_vec.ptr_mut(), lhs.data(), lhs.size());
	std::memcpy(rhs_vec.ptr_mut(), rhs.data(), rhs.size());

	failed_asserts.push({
			false,
			ByteStringView{"", 0},
			empty_str,
			op,
			VEG_FWD(lhs_vec),
			VEG_FWD(rhs_vec),
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
