#include "veg/util/assert.hpp"
#include "veg/util/timer.hpp"
#include "veg/internal/parse.hpp"
#include <cinttypes>
#include <iostream>
#include <memory>
#include <limits>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <exception>
#include <string>
#include <vector>
#include <new>
#include <chrono>
#include "veg/internal/prologue.hpp"

#ifdef __unix__
#include <unistd.h>

#define ISATTY ::isatty
#define FILENO ::fileno
#else
#include <io.h>

#define ISATTY ::_isatty
#define FILENO ::_fileno
#endif

namespace veg {
namespace internal {
namespace type_parse {
void function_decl_to_str(RefMut<std::string> str, FunctionDecl decl) noexcept;
} // namespace type_parse
[[noreturn]] void terminate() VEG_ALWAYS_NOEXCEPT {
	std::terminate();
}
} // namespace internal
namespace abi {
inline namespace VEG_ABI_VERSION {

namespace internal {
String::~String() {
	std::free(self.ptr);
}

void String::eprint() const VEG_ALWAYS_NOEXCEPT {
	std::cerr.write(self.ptr, isize(self.len));
	std::cerr.put('\n');
}
void String::reserve(usize new_cap) {
	if (new_cap > self.cap) {
		auto* new_alloc = static_cast<char*>(std::realloc(self.ptr, new_cap));
		if (new_alloc == nullptr) {
			throw std::bad_alloc{};
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

[[noreturn]] void terminate() VEG_ALWAYS_NOEXCEPT {
	std::terminate();
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

#define LIT(x)                                                                 \
	ByteStringView { x, sizeof(x) - 1 }

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
	s[i++] = '0' + c.r / 100;
	s[i++] = '0' + (c.r % 100) / 10;
	s[i++] = '0' + (c.r % 10);
	++i;

	s[i++] = '0' + c.g / 100;
	s[i++] = '0' + (c.g % 100) / 10;
	s[i++] = '0' + (c.g % 10);
	++i;

	s[i++] = '0' + c.b / 100;
	s[i++] = '0' + (c.b % 100) / 10;
	s[i++] = '0' + (c.b % 10);

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

thread_local std::vector<assertion_data>
		failed_asserts // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
		= {};

cleanup::~cleanup() VEG_ALWAYS_NOEXCEPT {
	failed_asserts.clear();
}

auto split_at(ByteStringView& text, usize n) -> ByteStringView {
	ByteStringView token = {text.data_, n};
	text = {text.data_ + n, text.len_ - n};
	return token;
}

auto starts_tk1(ByteStringView text, bool extended_char_set = false) -> bool {
	return (
			(!extended_char_set &&          //
	     (text.starts_with(LIT("<")) || //
	      text.starts_with(LIT(">")) || //
	      text.starts_with(LIT("{")) || //
	      text.starts_with(LIT("}")) || //
	      text.starts_with(LIT("(")) || //
	      text.starts_with(LIT(")")) || //
	      text.starts_with(LIT(" "))    //
	      )) ||                         //
			text.starts_with(LIT("[")) ||   //
			text.starts_with(LIT("]")) ||   //
			text.starts_with(LIT("=")) ||   //
			text.starts_with(LIT(",")) ||   //
			text.starts_with(LIT(";")) ||   //
			text.starts_with(LIT("*")) ||   //
			text.starts_with(LIT("&"))      //
	);
}

auto starts_tk2(ByteStringView text) -> bool {
	return (
			text.starts_with(LIT("->")) || //
			text.starts_with(LIT("&&")) || //
			text.starts_with(LIT("::")));
}

enum kind_e {
	whitespace,
	keyword,
	ident,
	qual,
	other,
};

struct token_t {
	ByteStringView text;
	kind_e kind;
};

auto split_if_starts_with(ByteStringView& code_str, ByteStringView word)
		-> ByteStringView {
	if (code_str.starts_with(word)) {
		return split_at(code_str, word.size());
	}
	return empty_str;
}

auto next_tk(ByteStringView& code_str, bool extended_char_set = false)
		-> token_t {
	// trim initial spaces
	while (code_str.starts_with(LIT(" "))) {
		code_str = {code_str.data() + 1, code_str.size() - 1};
	}

	ByteStringView matching = {" ", 1};
	if (extended_char_set) {
		if (code_str.starts_with(LIT("<"))) {
			matching = {">", 1};
		} else if (code_str.starts_with(LIT("{"))) {
			matching = {"}", 1};
		} else if (code_str.starts_with(LIT("("))) {
			matching = {")", 1};
		} else {
			extended_char_set = false;
		}
	}

	if (code_str.size() == 0) {
		return {code_str, other};
	}
	if (starts_tk2(code_str)) {
		return {split_at(code_str, 2), other};
	}

	if (starts_tk1(code_str, extended_char_set)) {
		return {split_at(code_str, 1), other};
	}

#define CONCAT2(A, B) A##B
#define CONCAT(A, B) CONCAT2(A, B)

#define STR2(A) #A
#define STR(A) STR2(A)

	ByteStringView word{nullptr, 0};
#define TOKEN_RETURN(Word, Kind)                                               \
	if ((word =                                                                  \
	         split_if_starts_with(code_str, {STR(Word), sizeof(STR(Word)) - 1})) \
	        .size() > 0) {                                                       \
		return {word, Kind};                                                       \
	}                                                                            \
	(void)0

	TOKEN_RETURN(noexcept, keyword);

	if (code_str.starts_with(LIT("__cdecl"))) {
		code_str = {code_str.data() + 7, code_str.size() - 7};
		return {{"", 0}, other};
	}

	TOKEN_RETURN(const, qual);
	TOKEN_RETURN(volatile, qual);
	TOKEN_RETURN(mutable, qual);
	TOKEN_RETURN(struct, qual);
	TOKEN_RETURN(typename, qual);
	TOKEN_RETURN(template, qual);

	TOKEN_RETURN(static, keyword);
	TOKEN_RETURN(inline, keyword);
	TOKEN_RETURN(extern, keyword);

	TOKEN_RETURN(constexpr, keyword);
	TOKEN_RETURN(virtual, keyword);

	TOKEN_RETURN(decltype(auto), ident);
	TOKEN_RETURN(auto, ident);

	TOKEN_RETURN(long double, ident);

	TOKEN_RETURN(unsigned long long int, ident);
	TOKEN_RETURN(unsigned long int, ident);
	TOKEN_RETURN(unsigned short int, ident);
	TOKEN_RETURN(unsigned long long, ident);
	TOKEN_RETURN(unsigned long, ident);
	TOKEN_RETURN(unsigned short, ident);

	TOKEN_RETURN(long long unsigned int, ident);
	TOKEN_RETURN(long unsigned int, ident);
	TOKEN_RETURN(short unsigned int, ident);
	TOKEN_RETURN(long long unsigned, ident);
	TOKEN_RETURN(long unsigned, ident);
	TOKEN_RETURN(short unsigned, ident);

	TOKEN_RETURN(long long int, ident);
	TOKEN_RETURN(long int, ident);
	TOKEN_RETURN(short int, ident);
	TOKEN_RETURN(long long, ident);
	TOKEN_RETURN(long, ident);
	TOKEN_RETURN(short, ident);

#undef TOKEN_RETURN

	ByteStringView new_str = code_str;
	while (
			!(new_str.size() == 0 || starts_tk1(new_str, extended_char_set) ||
	      starts_tk2(new_str))) {
		if (new_str.starts_with(matching)) {
			extended_char_set = false;
		}
		new_str = {new_str.data() + 1, new_str.size() - 1};
	}

	ByteStringView token = {
			code_str.data(), static_cast<usize>(new_str.data() - code_str.data())};
	code_str = new_str;
	return {token, ident};
}

auto peek_next_tk(ByteStringView code_str, bool extended_char_set = false)
		-> token_t {
	return next_tk(code_str, extended_char_set);
}

auto merge_tk(ByteStringView code_str, token_t prev_tk) -> ByteStringView {
	return {
			prev_tk.text.data(),
			static_cast<usize>(
					(code_str.data() + code_str.size()) - prev_tk.text.data())};
}

auto one_of(ByteStringView token, std::initializer_list<ByteStringView> tokens)
		-> bool {
	for (auto tk : tokens) { // NOLINT(readability-use-anyofallof)
		if (token == tk) {
			return true;
		}
	}
	return false;
}

using std::size_t;
enum scope_e { lparen, rparen, lsquare, rsquare, langle, rangle };

struct parse_type_result_t {
	std::vector<token_t> tokens = {};
	size_t max_nesting_count = 0;
	size_t num_nested = 0;
	bool multiline = false;
};

auto parse_type(ByteStringView& code_str) -> parse_type_result_t {

	constexpr token_t newline = {{"\n", 1}, whitespace};
	constexpr token_t indent = {{"| ", 2}, whitespace};

	struct state { // NOLINT(cppcoreguidelines-pro-type-member-init)
		size_t indent_level;
		size_t num_nested;
		token_t token;
		bool need_newline;
		parse_type_result_t res;
		bool entering;
	};
	std::vector<state> stack;
	stack.push_back({0, 0, {empty_str, {}}, false, {}, true});

	while (true) {
		bool continue_ = false;
		char const* begin = code_str.data();

		auto* state = &stack.back();
		bool entering = state->entering;

		if (entering) {
			state->res.num_nested = state->num_nested;
		}

		auto add_ln = [&] {
			state->res.multiline = true;
			state->res.tokens.push_back(newline);
			for (size_t i = 0; i < state->indent_level; ++i) {
				state->res.tokens.push_back(indent);
			}
		};

		token_t previous_token{empty_str, other};
		auto token = entering ? next_tk(code_str, true) : state->token;

		for (; !entering || token.text.size() > 0;
		     (previous_token = token, token = next_tk(code_str))) {

			if (!entering || token.text == ByteStringView{"<", 1}) {
				// if token comes after space, assume it's an identifier instead of
				// template syntax

				if (entering) {
					if (token.text.data() > begin && *(token.text.data() - 1) == ' ') {
						code_str = merge_tk(code_str, token);
						++(state->res.num_nested);
						state->entering = false;
						state->token = token;
						continue_ = true;
						break;
					}

					state->res.tokens.push_back(token);
				}

				bool need_newline = state->need_newline;
				if (entering) {
					++state->indent_level;
				}
				while (!entering || (token.text.size() > 0 &&
				                     !(token.text == ByteStringView{">", 1}))) {

					if (entering) {
						stack.push_back({state->indent_level, 0, token, false, {}, true});
						continue_ = true;
						break;
					}

					auto nested = VEG_MOV(state->res);
					stack.pop_back();
					if (stack.empty()) {
						return nested;
					}
					state = &stack.back();
					entering = true;

					state->res.multiline = state->res.multiline || nested.multiline;

					if (nested.tokens.empty()) {
						state->res = {};
						state->entering = false;
						state->token = token;
						state->need_newline = need_newline;
						continue_ = true;
						break;
					}

					state->res.max_nesting_count = std::max(
							state->res.max_nesting_count, nested.max_nesting_count + 1);
					state->res.num_nested += nested.num_nested;

					token = next_tk(code_str);

					if (state->res.num_nested > 1 ||
					    one_of(token.text, {{",", 1}, {";", 1}})) {
						need_newline = true;
						add_ln();
					}

					state->res.tokens.insert(
							state->res.tokens.end(),
							nested.tokens.begin(),
							nested.tokens.end());

					if (one_of(token.text, {{",", 1}, {";", 1}})) {
						state->res.tokens.push_back(token);
					}
				}
				if (continue_) {
					break;
				}
				--state->indent_level;

				if (token.text.size() == 0) {
					state->res = {};
					state->entering = false;
					state->token = token;
					state->need_newline = need_newline;
					continue_ = true;
					break;
				}

				if (need_newline) {
					add_ln();
				}
				state->res.tokens.push_back(token);
				if (peek_next_tk(code_str).text == LIT("::")) {
					continue;
				}
				state->entering = false;
				state->token = token;
				state->need_newline = need_newline;
				continue_ = true;
				break;

			} else if (
					one_of(
							token.text,
							{LIT("{"),
			         LIT("}"),
			         LIT("("),
			         LIT("["),
			         LIT(")"),
			         LIT("]"),
			         LIT(","),
			         LIT(";"),
			         LIT(">")}) ||
					(token.kind == ident &&
			     (previous_token.kind == ident ||
			      one_of(
								previous_token.text,
								{LIT("*"), LIT(" "), LIT("&"), LIT("&&")})))) {

				code_str = merge_tk(code_str, token);
				++state->res.num_nested;
				state->entering = false;
				state->token = token;
				continue_ = true;
				break;
			}
			if (token.text == LIT("::")) {
				state->res.num_nested = 0;
			}
			state->res.tokens.push_back(token);
		}
		if (continue_) {
			continue;
		}
		state->entering = false;
		state->token = token;
	}
	terminate();
}

auto to_owned(ByteStringView ref) -> std::string {
	return {ref.data(), static_cast<std::size_t>(ref.size())};
}

void print_type(
		std::string& output, parse_type_result_t const& type, color_t c) {

	token_t prev_tk = {empty_str, other};
	for (auto tk : type.tokens) {
		if (tk.text == LIT("::")) {
			output += with_color(c, to_owned(tk.text));
		} else {
			auto needs_space = [](token_t tok) -> bool {
				return tok.kind == qual || tok.kind == ident || tok.kind == keyword;
			};
			bool add_space = needs_space(tk) && needs_space(prev_tk);

			if (add_space) {
				output += ' ';
			}
			output += to_owned(tk.text);
		}
		prev_tk = tk;
	}
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
	output += with_color(orange_red, std::to_string(failed_asserts.size()));
	output += " assertion";
	if (failed_asserts.size() > 1) {
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
		veg::internal::type_parse::function_decl_to_str(
				mut(output),
				veg::internal::type_parse::parse_function_decl({
						from_raw_parts,
						{func.data(), func.size()},
				}));
		output += '\n';
	}

	char const* separator = "";

	for (auto const& a : failed_asserts) {
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
	std::cerr << on_fail(line, file, func, false);
}

[[noreturn]] void
on_assert_fail(long line, ByteStringView file, ByteStringView func) {
	std::cerr << on_fail(line, file, func, false);
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

	failed_asserts.push_back({
			false,
			LIT(""),
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
	failed_asserts.back().finished_setup = true;
	failed_asserts.back().expr = expression;
	failed_asserts.back().callback = msg;
}

auto snprintf1(char* out, usize n, unsigned type, void* arg) -> usize {
	unsigned type_id = type % 4U;
	unsigned metadata = type / 4U;

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
		int precision = 0;
		switch (metadata) {
		case sizeof(float):
			precision = int(std::numeric_limits<float>::max_digits10);
			break;
		case sizeof(double):
			precision = int(std::numeric_limits<double>::max_digits10);
			break;
		case sizeof(long double):
			precision = int(std::numeric_limits<long double>::max_digits10);
			break;
		default: {
		}
		}
		return usize(std::snprintf(
				out, n, "%.*Le", precision, *static_cast<long double*>(arg)));
	}
		HEDLEY_FALL_THROUGH;
	default:
		terminate();
	}
}
} // namespace internal
} // namespace VEG_ABI_VERSION
} // namespace abi
} // namespace veg
#include "veg/internal/epilogue.hpp"
