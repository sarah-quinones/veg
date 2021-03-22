#include "veg/assert.hpp"
#include "veg/timer.hpp"
#include <cinttypes>
#include <memory>

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
namespace abi {
inline namespace VEG_ABI_VERSION {

namespace internal {
string::~string() {
	std::free(self.ptr);
}
void string::reserve(i64 new_cap) {
	if (new_cap > self.cap) {
		auto* new_alloc = static_cast<char*>(std::realloc(self.ptr, new_cap));
		if (new_alloc == nullptr) {
			throw std::bad_alloc{};
		}
		self.ptr = new_alloc;
		self.cap = new_cap;
	}
}
void string::resize(i64 new_len) {
	reserve(new_len);
	self.len = new_len;
}
void string::insert(i64 pos, char const* data_, i64 len) {
	i64 old_size = size();
	i64 pre_new_size = len + old_size;
	i64 new_size =
			(pre_new_size > (2 * old_size)) ? pre_new_size : (2 * old_size);
	resize(new_size);
	if (len > 0) {
		std::memmove(data() + pos + len, data() + pos, std::size_t(old_size - pos));
		std::memmove(data() + pos, data_, std::size_t(len));
	}
}

[[noreturn]] void terminate() noexcept {
	std::terminate();
}
thread_local i64
		counter = // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
		0;
void incr_counter() noexcept {
	if (counter > 0) {
		std::fputs(
				"assertion failed during the handling of another failed assertion\n",
				stderr);
		terminate();
	}
	++counter;
}
void decr_counter() noexcept {
	--counter;
}

auto char_string_ref::starts_with(char_string_ref other) const noexcept
		-> bool {
	return size() >= other.size() &&
				 std::memcmp(data(), other.data(), other.size()) == 0;
}
auto char_string_ref::operator==(char_string_ref other) const noexcept -> bool {
	return size() == other.size() &&
				 std::memcmp(data(), other.data(), other.size()) == 0;
}
char_string_ref::char_string_ref(char const* str) noexcept
		: char_string_ref{str, static_cast<i64>(std::strlen(str))} {}

#define LIT(x)                                                                 \
	char_string_ref { x, sizeof(x) - 1 }

struct color_t {
	std::uint8_t r;
	std::uint8_t g;
	std::uint8_t b;
};

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
	char_string_ref expr;
	char_string_ref callback;
	char_string_ref op;
	std::string lhs;
	std::string rhs;
};

thread_local std::vector<assertion_data>
		failed_asserts // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
		= {};

cleanup::~cleanup() noexcept {
	failed_asserts.clear();
}

auto split_at(char_string_ref& text, i64 n) -> char_string_ref {
	char_string_ref token = {text.data_, n};
	text = {text.data_ + n, text.len_ - n};
	return token;
}

auto starts_tk1(char_string_ref text, bool extended_char_set = false) -> bool {
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

auto starts_tk2(char_string_ref text) -> bool {
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
	char_string_ref text;
	kind_e kind;
};

auto split_if_starts_with(char_string_ref& code_str, char_string_ref word)
		-> char_string_ref {
	if (code_str.starts_with(word)) {
		return split_at(code_str, word.size());
	}
	return empty_str;
}

auto next_tk(char_string_ref& code_str, bool extended_char_set = false)
		-> token_t {
	// trim initial spaces
	while (code_str.starts_with(LIT(" "))) {
		code_str = {code_str.data() + 1, code_str.size() - 1};
	}

	char_string_ref matching = {" ", 1};
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

	char_string_ref word{nullptr, 0};
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

	char_string_ref new_str = code_str;
	while (
			!(new_str.size() == 0 || starts_tk1(new_str, extended_char_set) ||
	      starts_tk2(new_str))) {
		if (new_str.starts_with(matching)) {
			extended_char_set = false;
		}
		new_str = {new_str.data() + 1, new_str.size() - 1};
	}

	char_string_ref token = {code_str.data(), new_str.data() - code_str.data()};
	code_str = new_str;
	return {token, ident};
}

auto peek_next_tk(char_string_ref code_str, bool extended_char_set = false)
		-> token_t {
	return next_tk(code_str, extended_char_set);
}

auto merge_tk(char_string_ref code_str, token_t prev_tk) -> char_string_ref {
	return {
			prev_tk.text.data(),
			code_str.data() + code_str.size() - prev_tk.text.data()};
}

auto one_of(
		char_string_ref token, std::initializer_list<char_string_ref> tokens)
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

auto parse_type(char_string_ref& code_str) -> parse_type_result_t {

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

			if (!entering || token.text == char_string_ref{"<", 1}) {
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
				                     !(token.text == char_string_ref{">", 1}))) {

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

auto to_owned(char_string_ref ref) -> std::string {
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

auto parse_func_signature(char_string_ref func) -> std::string {
	std::string output;
	auto color = azure;

	char_string_ref code_str = func;

	token_t token{empty_str, {}};
	while (peek_next_tk(code_str).kind == keyword) {
		token = next_tk(code_str);
	}

	bool trailing_return_type = false;

	if (peek_next_tk(code_str).text == LIT("auto")) {
		auto copy = code_str;
		next_tk(copy);
		if (!(peek_next_tk(copy).kind == qual ||
		      one_of(peek_next_tk(copy).text, {LIT("&"), LIT("&&"), LIT("*")}))) {
			trailing_return_type = true;
			code_str = copy;
			(void)trailing_return_type;
		}
	}

	parse_type_result_t return_type;
	std::vector<parse_type_result_t> fn_params;
	std::vector<token_t> fn_qual;

	std::vector<parse_type_result_t> tp_param_types;
	std::vector<token_t> tp_param_ids;
	std::vector<parse_type_result_t> tp_param_values;

	if (!trailing_return_type) {
		return_type = parse_type(code_str);
	}

	auto fn_name = parse_type(code_str);

	token = next_tk(code_str);
	if (!(token.text == LIT("("))) {
		return "";
	}

	// function parameters
	bool first_param = true;
	while (!(token.text.size() == 0 || token.text.starts_with(LIT(")")))) {
		auto res = parse_type(code_str);
		if (res.tokens.empty() && !first_param) {
			return "";
		}
		if (!res.tokens.empty()) {
			fn_params.push_back(static_cast<parse_type_result_t&&>(res));
		}
		token = next_tk(code_str);
		first_param = false;
	}

	if (token.text.size() == 0) {
		return "";
	}

	// member function qualifiers
	token = next_tk(code_str);
	while (token.kind == qual || token.text == LIT("noexcept")) {
		fn_qual.push_back(token);
		token = next_tk(code_str);
	}

	// return type
	if (trailing_return_type) {
		if (token.text == LIT("->")) {
			return_type = parse_type(code_str);
		}
		token = next_tk(code_str);
	}

	// template arguments
	if (token.text == LIT("[")) {
		if (peek_next_tk(code_str, true).text == LIT("with")) {
			token = next_tk(code_str);
		}

		first_param = true;
		while (!(token.text.size() == 0 || token.text.starts_with(LIT("]")))) {
			bool id_and_eq = false;
			{
				char_string_ref copy = code_str;
				next_tk(copy, true);
				auto eq = next_tk(copy);

				if (eq.text == LIT("=")) {
					id_and_eq = true;
				}
			}

			if (id_and_eq) {
				tp_param_types.emplace_back();
			} else {
				// assume type prefix
				auto res = parse_type(code_str);
				if (res.tokens.empty()) {
					return "";
				}
				tp_param_types.push_back(static_cast<parse_type_result_t&&>(res));
			}

			// id
			token = next_tk(code_str, true);
			tp_param_ids.push_back(token);

			// '=' sign
			token = next_tk(code_str);

			auto res = parse_type(code_str);
			if (res.tokens.empty() && !first_param) {
				return "";
			}
			tp_param_values.push_back(static_cast<parse_type_result_t&&>(res));
			token = next_tk(code_str);
			first_param = false;
		}
	}

	if (tp_param_ids.size() != tp_param_types.size() ||
	    tp_param_ids.size() != tp_param_values.size()) {
		return "";
	}

	output += '\n';

	output += with_color(color, "function name:");
	output += fn_name.multiline ? '\n' : ' ';
	print_type(output, fn_name, color);
	output += '\n';

	output += with_color(color, "return type =");
	output += return_type.multiline ? '\n' : ' ';
	print_type(output, return_type, color);
	output += '\n';

	if (!fn_qual.empty()) {
		output += with_color(color, "function qualifiers: ");
		char const* sep = "";
		for (auto tk : fn_qual) {
			output += with_color(color, sep + to_owned(tk.text));
			sep = ", ";
		}
		output += '\n';
	}

	for (auto const& param : fn_params) {
		output += with_color(color, "parameter type =");
		output += param.multiline ? '\n' : ' ';
		print_type(output, param, color);
		output += '\n';
	}

	for (size_t i = 0; i < tp_param_ids.size(); ++i) {
		auto& id = tp_param_ids[i];
		auto& type = tp_param_types[i];
		auto& value = tp_param_values[i];

		output += with_color(color, "template parameter");
		output += ' ';
		output += to_owned(id.text);
		if (type.tokens.empty()) {
		} else {
			output += with_color(color, " with type =");
			output += type.multiline ? '\n' : ' ';
			print_type(output, type, color);
			output += type.multiline ? '\n' : ' ';
			output += with_color(color, "and value");
		}
		output += with_color(color, " =");
		output += value.multiline ? '\n' : ' ';

		print_type(output, value, color);
		output += '\n';
	}
	output += '\n';
	return output;
}

auto find(char_string_ref sv, char c) -> char const* {
	char const* it = sv.data();
	for (; it < sv.data() + sv.size(); ++it) {
		if (*it == c) {
			break;
		}
	}
	return it;
}

auto on_fail(
		long line, char_string_ref file, char_string_ref func, bool is_fatal)
		-> std::string {
	auto _clear = [&] { failed_asserts.clear(); };
	auto&& clear = make::defer(_clear);

	std::string output;

	output += '\n';
	output += with_color(
			olive,
			"========================================"
			"=======================================");
	output += '\n';
	output += with_color(azure, std::to_string(failed_asserts.size()));
	output += " assertion";
	if (failed_asserts.size() > 1) {
		output += 's';
	}
	output += ' ';
	output += with_color(orange_red, "failed");
	output += '\n';
	if (func.size() > 0) {
		output += "in function:\n";
		output += to_owned(func);
		output += '\n';
		output +=
				with_color(gray, to_owned(file) + (':' + std::to_string(line)) + ':');
		output += '\n';
		output += parse_func_signature(func);
	}

	char const* separator = "";

	for (auto const& a : failed_asserts) {
		char_string_ref msg{a.callback};
		char const* newline = find(msg, '\n');
		bool multiline = newline != nullptr;

		output += separator;

		if (is_fatal) {
			output += with_color(orange_red, "fatal ");
		}
		output += "assertion ";
		output += '`';
		output += with_color(azure, to_owned(a.expr));
		output += '\'';
		output += with_color(orange_red, " failed:");

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
					e = find({b, (msg.data() + msg.size()) - b}, '\n');
				}
			}
		}

		output += "\nassertion expands to: `";
		output += with_color(azure, a.lhs + to_owned(a.op) + a.rhs);
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

void on_expect_fail(long line, char_string_ref file, char_string_ref func) {
	std::fputs(on_fail(line, file, func, false).c_str(), stderr);
}

[[noreturn]] void
on_assert_fail(long line, char_string_ref file, char_string_ref func) {
	std::fputs(on_fail(line, file, func, false).c_str(), stderr);
	std::terminate();
}

void set_assert_params1( //
		char_string_ref op,  //
		string lhs,          //
		string rhs           //
) {
	bool success = false;

	auto _clear = [&] {
		if (!success) {
			failed_asserts.clear();
		}
	};

	auto&& clear = make::defer(_clear);

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
void set_assert_params2(        //
		char_string_ref expression, //
		char_string_ref msg         //
		) noexcept {
	failed_asserts.back().finished_setup = true;
	failed_asserts.back().expr = expression;
	failed_asserts.back().callback = msg;
}

// if possible:
// aligns the pointer
// then advances it by `size` bytes, and decreases `space` by `size`
// returns the previous aligned value
//
// otherwise, if there is not enough space for aligning or advancing the
// pointer, returns nullptr and the values are left unmodified
auto align_next(i64 alignment, i64 size, void*& ptr, i64& space) noexcept
		-> void* {
	static_assert(
			sizeof(std::uintptr_t) >= sizeof(void*),
			"std::uintptr_t can't hold a pointer value");

	using byte_ptr = unsigned char*;

	// assert alignment is power of two
	VEG_ASSERT_ALL_OF(
			(alignment > 0), ((u64(alignment) & (u64(alignment) - 1)) == 0));

	if (space < size) {
		return nullptr;
	}

	std::uintptr_t lo_mask = alignment - 1;
	std::uintptr_t hi_mask = ~lo_mask;

	auto const intptr = reinterpret_cast<std::uintptr_t>(ptr);
	auto* const byteptr = static_cast<byte_ptr>(ptr);

	auto offset = ((intptr + alignment - 1) & hi_mask) - intptr;

	if (space - size < offset) {
		return nullptr;
	}

	void* const rv = byteptr + offset;

	ptr = byteptr + (offset + size);
	space = space - (offset + size);

	return rv;
}
auto vsnprintf(char* out, usize n, char const* fmt, va_list args) -> int {
	return std::vsnprintf(out, n, fmt, args);
}
} // namespace internal
} // namespace VEG_ABI_VERSION
} // namespace abi
} // namespace veg
#include "veg/internal/epilogue.hpp"
