#include "veg/tuple.hpp"
#include "veg/option.hpp"
#include <veg/uwunion.hpp>
#include <veg/functional/bind.hpp>
#include <veg/slice.hpp>
#include <vector>
#include <fmt/core.h>
#include <cassert>
#include <cstring>

#define PRINT_FN() fmt::print("{}\n", __PRETTY_FUNCTION__)

namespace veg {

template <typename T>
void dbg_slice(fmt::Buffer& out, Slice<T> s) {
	fmt::DbgStructScope _{out};

	for (isize i = 0; i < s.len(); ++i) {
		out.append_ln();
		fmt::dbg_to(out, ref(s.as_ptr()[i]));
		out.append_literal(",");
	}
}

template <typename T>
struct Box {
private:
	T* inner;

public:
	VEG_INLINE Box(Direct /*tag*/, T value) noexcept(false)
			: inner{new T(VEG_FWD(value))} {}
	template <typename Fn>
	VEG_INLINE Box(InPlace /*tag*/, Fn fn) noexcept(false)
			: inner{new T(VEG_FWD(fn)())} {}

	VEG_INLINE explicit Box(Box const& rhs) noexcept(false)
			: inner{new T(*(rhs.inner))} {}
	VEG_INLINE Box(Box&& rhs) noexcept : inner{rhs.inner} { rhs.inner = {}; }

	VEG_INLINE auto operator=(Box const& rhs) noexcept(false) -> Box& {
		if (this != mem::addressof(rhs)) {
			if (inner == nullptr) {
				*this = Box(rhs);
			} else {
				inner = rhs.inner;
			}
		}
		return *this;
	}
	VEG_INLINE auto operator=(Box&& rhs) noexcept(true) -> Box& {
		T* tmp = rhs.inner;
		Box cleanup = static_cast<Box&&>(*this);
		rhs.inner = {};
		inner = tmp;
		return *this;
	}

	auto operator->() const noexcept -> T const* { return inner; }
	auto operator->() noexcept -> T* { return inner; }

	VEG_NODISCARD auto ref() const noexcept -> Ref<T> { return veg::ref(*inner); }
	VEG_NODISCARD auto mut() noexcept -> RefMut<T> { return veg::mut(*inner); }

	~Box() { delete inner; }
};
template <typename T, typename U>
auto operator==(Box<T> const& lhs, Box<U> const& rhs) -> bool {
	return lhs.ref().get() == rhs.ref().get();
}

namespace nb {
struct box {
	template <typename T>
	VEG_INLINE auto operator()(T val) const -> Box<T> {
		return {direct, VEG_FWD(val)};
	}
};
} // namespace nb
VEG_NIEBLOID(box);

template <typename T>
struct fmt::Debug<Box<T>> {
	static void to_string(fmt::Buffer& out, Ref<Box<T>> val) {
		dbg_to(out, val.get().ref());
	}
};

namespace type_parse {
VEG_TAG(from_literal, FromLiteral);
struct StrView {
	struct Inner {
		char const* begin;
		usize len;
	} _{};

	StrView() = default;

	constexpr StrView(FromRawParts /*tag*/, Inner inner) noexcept : _{inner} {}

	template <usize N>
	constexpr StrView(FromLiteral /*tag*/, char const (&literal)[N]) noexcept
			: _{&literal[0], N - 1} {}

	VEG_NODISCARD constexpr auto ptr() const noexcept -> char const* {
		return _.begin;
	}
	VEG_NODISCARD constexpr auto len() const noexcept -> usize { return _.len; }
	VEG_NODISCARD constexpr auto split_at(usize pos) const noexcept
			-> Tuple<StrView, StrView> {

		assert(pos <= _.len);
		return {
				direct,
				{from_raw_parts, {_.begin, pos}},
				{from_raw_parts, {_.begin + pos, _.len - pos}},
		};
	}
	VEG_NODISCARD constexpr auto substr(usize idx, usize len) const noexcept
			-> StrView {
		return VEG_ASSERT_ALL_OF(
							 (idx <= this->len()), //
							 (idx + len <= this->len())),
		       StrView{from_raw_parts, {_.begin + idx, len}};
	}
	VEG_NODISCARD constexpr auto head(usize len) const noexcept -> StrView {
		return substr(0, len);
	}
	VEG_NODISCARD constexpr auto tail(usize len) const noexcept -> StrView {
		return assert(len <= this->len()), substr(this->len() - len, len);
	}
	VEG_NODISCARD constexpr auto skip_leading(usize n) const noexcept -> StrView {
		return substr(n, len() - n);
	}

	VEG_NODISCARD constexpr auto begins_with(StrView other) const noexcept
			-> bool {
		return (other.len() <= len()) &&
		       (std::memcmp(ptr(), other.ptr(), other.len()) == 0);
	}
	VEG_NODISCARD constexpr auto ends_with(StrView other) const noexcept -> bool {
		return (other.len() <= len()) &&
		       (std::memcmp(
								ptr() + (len() - other.len()), other.ptr(), other.len()) == 0);
	}
	VEG_NODISCARD auto eq(StrView other) const noexcept -> bool {
		return (other.len() == len()) &&
		       (std::memcmp(ptr(), other.ptr(), len()) == 0);
	}
	VEG_NODISCARD constexpr auto ltrim(char c) const noexcept -> StrView {
		usize pos = 0;
		while (true) {
			if (pos == len() || ptr()[pos] != c) {
				break;
			}
			++pos;
		}
		return split_at(pos)[1_c];
	}
	VEG_NODISCARD constexpr auto rtrim(char c) const noexcept -> StrView {
		usize pos = len();
		while (true) {
			if (pos == 0 || (ptr()[pos - 1] != c)) {
				break;
			}
			--pos;
		}
		return split_at(pos)[0_c];
	}
	VEG_NODISCARD constexpr auto trim(char c) const noexcept -> StrView {
		return ltrim(c).rtrim(c);
	}
};
} // namespace type_parse

template <>
struct fmt::Debug<type_parse::StrView> {
	static void to_string(fmt::Buffer& out, Ref<type_parse::StrView> str_ref) {
		out.reserve(str_ref->len() + 2);
		usize n = out.size();
		// TODO: escape sequences
		char quotes = '\"';
		auto str = str_ref.get();
		out.insert(n, &quotes, 1);
		out.insert(n + 1, str.ptr(), str.len());
		out.insert(n + 1 + str.len(), &quotes, 1);
	}
};

namespace type_parse {
auto operator==(StrView a, StrView b) noexcept {
	return a.eq(b);
}

struct Entity;

enum struct CvQual : unsigned char {
	NONE,
	CONST,
	VOLATILE,
	CONST_VOLATILE,
};
enum struct RefQual : unsigned char {
	LVALUE,
	RVALUE,
	POINTER,
};

struct EntityName {
	StrView name;
};

struct TemplatedEntity {
	Box<Entity> tpl;
	std::vector<Entity> args;
};
struct NestedEntity {
	std::vector<Entity> components;
};

struct Pointer {
	Box<Entity> entity;
	RefQual ref_qual;
};
struct Array {
	Box<Entity> entity;
	Box<Entity> size;
};
struct Function {
	Box<Entity> return_type;
	std::vector<Entity> args;
};

struct Entity {
	using Uwunion = veg::Uwunion<
			EntityName,
			TemplatedEntity,
			NestedEntity,

			Pointer,
			Array,
			Function>;

	Uwunion kind;
	CvQual cv_qual;
};
auto operator==(Entity const& lhs, Entity const& rhs) -> bool;

auto operator==(EntityName const& lhs, EntityName const& rhs) -> bool {
	return lhs.name == rhs.name;
}
auto operator==(TemplatedEntity const& lhs, TemplatedEntity const& rhs)
		-> bool {
	return lhs.tpl == rhs.tpl && lhs.args == rhs.args;
}
auto operator==(NestedEntity const& lhs, NestedEntity const& rhs) -> bool {
	return lhs.components == rhs.components;
}
auto operator==(Pointer const& lhs, Pointer const& rhs) -> bool {
	return lhs.entity == rhs.entity && lhs.ref_qual == rhs.ref_qual;
}
auto operator==(Array const& lhs, Array const& rhs) -> bool {
	return lhs.entity == rhs.entity && lhs.size == rhs.size;
}
auto operator==(Function const& lhs, Function const& rhs) -> bool {
	return lhs.return_type == rhs.return_type && lhs.args == rhs.args;
}

auto operator==(Entity const& lhs, Entity const& rhs) -> bool {
	return lhs.kind == rhs.kind && lhs.cv_qual == rhs.cv_qual;
}
} // namespace type_parse

namespace fmt {
template <>
struct Debug<type_parse::Entity> {
	static inline void to_string(fmt::Buffer& out, Ref<type_parse::Entity> e);
};
template <>
struct Debug<type_parse::EntityName> {
	static inline void
	to_string(fmt::Buffer& out, Ref<type_parse::EntityName> e) {
		dbg_to(out, ref(e->name));
	}
};
template <>
struct Debug<type_parse::TemplatedEntity> {
	static inline void
	to_string(fmt::Buffer& out, Ref<type_parse::TemplatedEntity> e) {

		out.append_literal("TemplatedEntity ");
		fmt::DbgStructScope _{out};

		out.append_ln();
		out.append_literal("Template: ");
		dbg_to(out, ref(e->tpl));

		out.append_ln();
		out.append_literal("Args: ");
		dbg_slice(
				out,
				Slice<type_parse::Entity>{
						from_raw_parts,
						e->args.data(),
						isize(e->args.size()),
						unsafe,
				});
	}
};
template <>
struct Debug<type_parse::NestedEntity> {
	static inline void
	to_string(fmt::Buffer& out, Ref<type_parse::NestedEntity> e) {
		out.append_literal("NestedEntity ");

		dbg_slice<type_parse::Entity>(
				out,
				{
						from_raw_parts,
						e->components.data(),
						isize(e->components.size()),
						unsafe,
				});
	}
};
template <>
struct Debug<type_parse::Pointer> {
	static inline void to_string(fmt::Buffer& out, Ref<type_parse::Pointer> e) {
		switch (e->ref_qual) {
		case type_parse::RefQual::POINTER:
			out.append_literal("*");
			break;
		case type_parse::RefQual::LVALUE:
			out.append_literal("&");
			break;
		case type_parse::RefQual::RVALUE:
			out.append_literal("&&");
			break;
		default:
			assert(false);
		}
		dbg_to(out, ref(e->entity));
	}
};

inline void Debug<type_parse::Entity>::to_string(
		fmt::Buffer& out, Ref<type_parse::Entity> e) {
	switch (e->cv_qual) {
	case type_parse::CvQual::NONE:
		break;
	case type_parse::CvQual::CONST:
		out.append_literal("const ");
		break;
	case type_parse::CvQual::VOLATILE:
		out.append_literal("mut volatile ");
		break;
	case type_parse::CvQual::CONST_VOLATILE:
		out.append_literal("const volatile ");
		break;
	default:
		assert(false);
	}
	dbg_to(out, ref(e->kind));
}
} // namespace fmt

namespace type_parse {

auto is_ident_char(char c) {
	return (c == '~') ||             //
	       (c == '_') ||             //
	       (c >= 'a' && c <= 'z') || //
	       (c >= 'A' && c <= 'Z') || //
	       (c >= '0' && c <= '9');
}

enum struct TokenKind {
	UNARY_OP,
	BINARY_OP,
	AMBIGUOUS,
	OPEN_DELIM,
	CLOSE_DELIM,
	COMPOSITE_PRIMITIVE,
	IDENT,
};
struct Token {
	StrView str;
	TokenKind kind;
};
} // namespace type_parse
template <>
struct fmt::Debug<type_parse::Token> {
	static void to_string(fmt::Buffer& out, Ref<type_parse::Token> r) {
		dbg_to(out, ref(r->str));
	}
};
namespace type_parse {
constexpr Token empty_tk = {{from_literal, ""}, TokenKind::IDENT};

constexpr Token basic_tokens[] = {
		{{from_literal, "<dependent type>"}, TokenKind::IDENT},
		{{from_literal, "operator()"}, TokenKind::IDENT},
		{{from_literal, "operator[]"}, TokenKind::IDENT},
		{{from_literal, "operator~"}, TokenKind::IDENT},

		{{from_literal, "operator++"}, TokenKind::IDENT},
		{{from_literal, "operator--"}, TokenKind::IDENT},

		{{from_literal, "operator+="}, TokenKind::IDENT},
		{{from_literal, "operator-="}, TokenKind::IDENT},
		{{from_literal, "operator*="}, TokenKind::IDENT},
		{{from_literal, "operator/="}, TokenKind::IDENT},
		{{from_literal, "operator^="}, TokenKind::IDENT},
		{{from_literal, "operator%="}, TokenKind::IDENT},
		{{from_literal, "operator|="}, TokenKind::IDENT},
		{{from_literal, "operator&="}, TokenKind::IDENT},
		{{from_literal, "operator<<="}, TokenKind::IDENT},
		{{from_literal, "operator>>="}, TokenKind::IDENT},

		{{from_literal, "operator+"}, TokenKind::IDENT},
		{{from_literal, "operator-"}, TokenKind::IDENT},
		{{from_literal, "operator*"}, TokenKind::IDENT},
		{{from_literal, "operator/"}, TokenKind::IDENT},
		{{from_literal, "operator^"}, TokenKind::IDENT},
		{{from_literal, "operator%"}, TokenKind::IDENT},
		{{from_literal, "operator|"}, TokenKind::IDENT},
		{{from_literal, "operator&"}, TokenKind::IDENT},
		{{from_literal, "operator<<"}, TokenKind::IDENT},
		{{from_literal, "operator>>"}, TokenKind::IDENT},

		{{from_literal, "operator,"}, TokenKind::IDENT},

		{{from_literal, "operator<=>"}, TokenKind::IDENT},
		{{from_literal, "operator<="}, TokenKind::IDENT},
		{{from_literal, "operator>="}, TokenKind::IDENT},
		{{from_literal, "operator<"}, TokenKind::IDENT},
		{{from_literal, "operator>"}, TokenKind::IDENT},
		{{from_literal, "operator=="}, TokenKind::IDENT},
		{{from_literal, "operator!="}, TokenKind::IDENT},

		{{from_literal, "operator="}, TokenKind::IDENT},

		{{from_literal, "operator!"}, TokenKind::IDENT},
		{{from_literal, "operator||"}, TokenKind::IDENT},
		{{from_literal, "operator&&"}, TokenKind::IDENT},

		{{from_literal, "operator->"}, TokenKind::IDENT},
		{{from_literal, "operator->*"}, TokenKind::IDENT},

		{{from_literal, "*"}, TokenKind::UNARY_OP},
		{{from_literal, "&&"}, TokenKind::BINARY_OP},
		{{from_literal, "&"}, TokenKind::UNARY_OP},
		{{from_literal, "::"}, TokenKind::BINARY_OP},
		{{from_literal, "["}, TokenKind::OPEN_DELIM},
		{{from_literal, "]"}, TokenKind::CLOSE_DELIM},
		{{from_literal, "("}, TokenKind::OPEN_DELIM},
		{{from_literal, ")"}, TokenKind::CLOSE_DELIM},
		{{from_literal, "{"}, TokenKind::OPEN_DELIM},
		{{from_literal, "}"}, TokenKind::CLOSE_DELIM},
		{{from_literal, "=="}, TokenKind::BINARY_OP},
		{{from_literal, "!="}, TokenKind::BINARY_OP},
		{{from_literal, "="}, TokenKind::BINARY_OP},
		{{from_literal, ">="}, TokenKind::BINARY_OP},
		{{from_literal, "<="}, TokenKind::BINARY_OP},
		{{from_literal, "<"}, TokenKind::AMBIGUOUS},
		{{from_literal, ">"}, TokenKind::AMBIGUOUS},

		{{from_literal, "unsigned char"}, TokenKind::COMPOSITE_PRIMITIVE},
		{{from_literal, "signed char"}, TokenKind::COMPOSITE_PRIMITIVE},
		{{from_literal, "char"}, TokenKind::COMPOSITE_PRIMITIVE},

		{{from_literal, "unsigned short int"}, TokenKind::COMPOSITE_PRIMITIVE},
		{{from_literal, "unsigned short"}, TokenKind::COMPOSITE_PRIMITIVE},
		{{from_literal, "short unsigned int"}, TokenKind::COMPOSITE_PRIMITIVE},
		{{from_literal, "short unsigned"}, TokenKind::COMPOSITE_PRIMITIVE},
		{{from_literal, "short int"}, TokenKind::COMPOSITE_PRIMITIVE},
		{{from_literal, "short"}, TokenKind::COMPOSITE_PRIMITIVE},

		{{from_literal, "unsigned long long int"}, TokenKind::COMPOSITE_PRIMITIVE},
		{{from_literal, "unsigned long long"}, TokenKind::COMPOSITE_PRIMITIVE},
		{{from_literal, "long long unsigned int"}, TokenKind::COMPOSITE_PRIMITIVE},
		{{from_literal, "long long unsigned"}, TokenKind::COMPOSITE_PRIMITIVE},
		{{from_literal, "long long int"}, TokenKind::COMPOSITE_PRIMITIVE},
		{{from_literal, "long long"}, TokenKind::COMPOSITE_PRIMITIVE},

		{{from_literal, "unsigned long int"}, TokenKind::COMPOSITE_PRIMITIVE},
		{{from_literal, "unsigned long"}, TokenKind::COMPOSITE_PRIMITIVE},
		{{from_literal, "long unsigned int"}, TokenKind::COMPOSITE_PRIMITIVE},
		{{from_literal, "long unsigned"}, TokenKind::COMPOSITE_PRIMITIVE},
		{{from_literal, "long int"}, TokenKind::COMPOSITE_PRIMITIVE},
		{{from_literal, "long"}, TokenKind::COMPOSITE_PRIMITIVE},

		{{from_literal, "unsigned int"}, TokenKind::COMPOSITE_PRIMITIVE},
		{{from_literal, "int unsigned"}, TokenKind::COMPOSITE_PRIMITIVE},
		{{from_literal, "unsigned"}, TokenKind::COMPOSITE_PRIMITIVE},
		{{from_literal, "int"}, TokenKind::COMPOSITE_PRIMITIVE},
};

auto find_matching(StrView c, StrView str, usize initial_count = 0) noexcept
		-> Tuple<StrView, StrView> {
	usize delimiter_count = initial_count;
	usize angled_delimiter_count = 0;
	usize pos = 0;
	for (; pos < str.len(); ++pos) {
		if (delimiter_count == 0 && angled_delimiter_count == 0 &&
		    str.skip_leading(pos).begins_with(c)) {
			break;
		}

		char cur = str.ptr()[pos];
		if (cur == '[' || cur == '(' || cur == '{') {
			++delimiter_count;
		}
		if (cur == ']' || cur == ')' || cur == '}') {
			--delimiter_count;
		}
		if (delimiter_count == 0) {
			if (cur == '<') {
				++angled_delimiter_count;
			}
			if (cur == '>') {
				--angled_delimiter_count;
			}
		}
	}
	return str.split_at(pos);
}

auto parse_token(StrView str) noexcept -> Tuple<Token, StrView> {
	str = str.ltrim(' ');
	for (auto basic_token : type_parse::basic_tokens) {
		if (str.begins_with(basic_token.str)) {
			auto res = str.split_at(basic_token.str.len());
			if (basic_token.kind != TokenKind::COMPOSITE_PRIMITIVE || //
			    res[1_c].len() == 0 ||                                //
			    !type_parse::is_ident_char(*res[1_c].ptr())) {
				return {
						direct,
						{res[0_c], basic_token.kind},
						res[1_c],
				};
			}
		}
	}

	if (str.begins_with({from_literal, "decltype"})) {
		VEG_BIND(
				auto,
				(head, tail),
				type_parse::find_matching({from_literal, ")"}, str, usize(-1)));
		assert(tail.len() != usize(0));
		return {
				direct,
				{str.head(head.len() + 1), TokenKind::IDENT},
				{str.tail(tail.len() - 1)},
		};
	}

	usize start_at = 0;
	if (str.begins_with({from_literal, "template "})) {
		start_at = StrView{from_literal, "template "}.len();
	}
	if (str.begins_with({from_literal, "typename "})) {
		start_at = StrView{from_literal, "typename "}.len();
	}

	usize pos = start_at;
	while (type_parse::is_ident_char(str.ptr()[pos])) {
		++pos;
	}
	VEG_BIND(auto, (tk, tail), str.split_at(pos));
	return {direct, {tk, TokenKind::IDENT}, tail};
}

auto greedy_parse_nestable_entity(StrView str) -> Tuple<Entity, StrView>;
auto greedy_parse_entity(StrView str) -> Tuple<Entity, StrView>;

auto parse_entity(StrView str) -> Entity {
	return type_parse::greedy_parse_entity(str)[0_c];
}

auto parse_templated_entity(StrView between_angled_brackets)
		-> std::vector<Entity> {
	std::vector<Entity> out;
	while (true) {
		VEG_BIND(
				auto,
				(head, tail),
				type_parse::find_matching(
						{from_literal, ","}, between_angled_brackets));

		head = head.trim(' ');
		if (head.len() == 0) {
			break;
		}

		out.push_back(type_parse::parse_entity(head));
		if (tail.len() == 0) {
			// no more commas
			// this was the last param
			break;
		}
		between_angled_brackets = tail.skip_leading(1);
	}
	return out;
}

auto greedy_parse_templated_entity(StrView after_angled)
		-> Tuple<std::vector<Entity>, StrView> {
	VEG_BIND( //
			auto,
			(between_angled, tail),
			type_parse::find_matching({from_literal, ">"}, after_angled));
	assert(tail.len() > usize(0));
	assert(tail.ptr()[0] == '>');
	return {
			direct,
			type_parse::parse_templated_entity(between_angled),
			tail.skip_leading(1),
	};
}

auto greedy_parse_nestable_entity(StrView str) -> Tuple<Entity, StrView> {
	VEG_BIND(auto, (tk, tail), type_parse::parse_token(str));

	if (tk.str == StrView({from_literal, "<"})) {
		tail = str;
		tk = empty_tk;
	}
	if (tail.begins_with({from_literal, "<"})) {

		Entity tpl_head = Entity{
				Entity::Uwunion{
						0_c,
						EntityName{tk.str},
				},
				CvQual::NONE,
		};

		VEG_BIND(
				auto,
				(tpl_args, tail2),
				type_parse::greedy_parse_templated_entity(tail.skip_leading(1)));

		return Tuple<Entity, StrView>{
				direct,
				Entity{
						Entity::Uwunion{
								1_c,
								TemplatedEntity{
										box(VEG_FWD(tpl_head)),
										VEG_FWD(tpl_args),
								},
						},
						CvQual::NONE,
				},
				tail2,
		};
	}
	return {
			direct,
			Entity{
					Entity::Uwunion{
							0_c,
							EntityName{tk.str},
					},
					CvQual::NONE,
			},
			tail};
}

auto greedy_parse_nested_entity(StrView str) -> Tuple<Entity, StrView> {
	// skip global scope
	{
		VEG_BIND(auto, (tk, after_tk), type_parse::parse_token(str));
		if (tk.str.eq({from_literal, "::"})) {
			str = after_tk;
		}
	}

	std::vector<Entity> v;
	while (true) {
		VEG_BIND(auto, (e, tail), type_parse::greedy_parse_nestable_entity(str));
		VEG_BIND(auto, (next_tk, after_token), type_parse::parse_token(tail));

		str = tail;
		v.push_back(VEG_FWD(e));
		if (next_tk.str.eq({from_literal, "::"})) {
			str = after_token;
		} else {
			break;
		}
	}
	if (v.size() == 1) {
		return {
				direct,
				static_cast<Entity&&>(v.front()),
				str,
		};
	}

	return {
			direct,
			Entity{
					Entity::Uwunion{
							2_c,
							NestedEntity{v},
					},
					CvQual::NONE,
			},
			str,
	};
}
auto apply_const(CvQual cv) noexcept -> CvQual {
	switch (cv) {
	case CvQual::NONE:
		return CvQual::CONST;
	case CvQual::VOLATILE:
		return CvQual::CONST_VOLATILE;
	default:
		return cv;
	}
}
auto apply_volatile(CvQual cv) noexcept -> CvQual {
	switch (cv) {
	case CvQual::NONE:
		return CvQual::VOLATILE;
	case CvQual::CONST:
		return CvQual::CONST_VOLATILE;
	default:
		return cv;
	}
}

auto greedy_parse_entity(StrView str) -> Tuple<Entity, StrView> {
	// if starts with parens, skip the whole thing
	{
		VEG_BIND(auto, (tk, after_token), type_parse::parse_token(str));
		if (tk.str.eq({from_literal, "("})) {
			VEG_BIND(
					auto,
					(inside_parens, after_parens),
					type_parse::find_matching({from_literal, ")"}, after_token));
			assert(after_parens.begins_with({from_literal, ")"}));
			inside_parens._.begin -= 1;
			inside_parens._.len += 2;

			return {
					direct,
					Entity{
							Entity::Uwunion{
									0_c,
									EntityName{inside_parens},
							},
							CvQual::NONE,
					},
					after_token.skip_leading(1),
			};
		}
	}
	auto cv_qual = CvQual::NONE;

	// nothing on the left, cv qualifiers apply to the entity on the right
	while (true) {
		VEG_BIND(auto, (tk, tail), type_parse::parse_token(str));
		if (tk.str.eq({from_literal, "const"})) {
			cv_qual = type_parse::apply_const(cv_qual);
			str = tail;
		} else if (tk.str.eq({from_literal, "volatile"})) {
			cv_qual = type_parse::apply_volatile(cv_qual);
			str = tail;
		} else {
			break;
		}
	}

	VEG_BIND(auto, (e, tail), type_parse::greedy_parse_nested_entity(str));
	e.cv_qual = cv_qual;

	while (true) {
		VEG_BIND(auto, (tk, tail2), type_parse::parse_token(tail));
		if (tk.str.eq({from_literal, "*"})) {
			e = Entity{
					Entity::Uwunion{
							3_c,
							Pointer{box(VEG_FWD(e)), RefQual::POINTER},
					},
					CvQual::NONE,
			};
			tail = tail2;
		} else if (tk.str.eq({from_literal, "&&"})) {
			e = Entity{
					Entity::Uwunion{
							3_c,
							Pointer{box(VEG_FWD(e)), RefQual::RVALUE},
					},
					CvQual::NONE,
			};
			tail = tail2;
		} else if (tk.str.eq({from_literal, "&"})) {
			e = Entity{
					Entity::Uwunion{
							3_c,
							Pointer{box(VEG_FWD(e)), RefQual::LVALUE},
					},
					CvQual::NONE,
			};
			tail = tail2;
		} else if (tk.str.eq({from_literal, "["})) {
			VEG_BIND(
					auto,
					(size_str, tail3),
					type_parse::find_matching({from_literal, "]"}, tail2));

			e = Entity{
					Entity::Uwunion{
							4_c,
							Array{
									box(VEG_FWD(e)),
									box(Entity{
											Entity::Uwunion{0_c, EntityName{size_str}},
											CvQual::NONE,
									}),
							},
					},
					CvQual::NONE,
			};
			assert(tail3.begins_with({from_literal, "]"}));
			tail = tail3.skip_leading(1);
		} else if (tk.str.eq({from_literal, "const"})) {
			// cv qualifiers apply to the entity on the left
			e.cv_qual = type_parse::apply_const(e.cv_qual);
			tail = tail2;
		} else if (tk.str.eq({from_literal, "volatile"})) {
			e.cv_qual = type_parse::apply_volatile(e.cv_qual);
			tail = tail2;
		} else {
			break;
		}
	}

	return {
			direct,
			VEG_FWD(e),
			tail,
	};
}

struct FunctionDecl {
	Option<Entity> return_type;
	Entity full_name;
	std::vector<Entity> args;
	CvQual cv_qual;
	bool is_static;
	std::vector<Tuple<Entity, Entity>> dependent_expansions;
};

auto operator==(FunctionDecl const& lhs, FunctionDecl const& rhs) -> bool {
	return lhs.return_type == rhs.return_type && lhs.full_name == rhs.full_name &&
	       lhs.args == rhs.args && lhs.cv_qual == rhs.cv_qual &&
	       lhs.is_static == rhs.is_static;
}

auto parse_function_decl(StrView str) -> FunctionDecl {
	bool is_static = false;

	// scan for static
	{
		VEG_BIND(auto, (maybe_static, after_static), type_parse::parse_token(str));
		if (maybe_static.str.eq({from_literal, "static"})) {
			is_static = true;
			str = after_static;
		}
	}

	// scan for return type and function name
	// if both are found, then parsed as "ret fn"
	// else, parsed as "fn"
	VEG_BIND(
			auto,
			(fn_name_or_return, after_name),
			type_parse::greedy_parse_entity(str));

	auto name_ret = [&]() -> Tuple<Entity, Option<Entity>> {
		if (after_name.begins_with({from_literal, "("})) {
			// ctor or dtor
			return {
					direct,
					VEG_FWD(fn_name_or_return),
					none,
			};
		} else {
			VEG_BIND( //
					auto,
					(actual_name, new_after_name),
					type_parse::greedy_parse_entity(after_name));

			after_name = new_after_name;

			return {
					direct,
					VEG_FWD(actual_name),
					some(VEG_FWD(fn_name_or_return)),
			};
		}
	}();

	assert(after_name.begins_with({from_literal, "("}));
	after_name = after_name.skip_leading(1);

	VEG_BIND( //
			auto,
			(between_angled, after_args),
			type_parse::find_matching({from_literal, ")"}, after_name));
	auto args = type_parse::parse_templated_entity(between_angled);

	assert(after_args.begins_with({from_literal, ")"}));
	after_args = after_args.skip_leading(1);

	// cv qualifiers
	CvQual cv = CvQual::NONE;
	while (true) {
		VEG_BIND(auto, (tk, tail), type_parse::parse_token(after_args));
		if (tk.str.eq({from_literal, "const"})) {
			cv = CvQual(unsigned(cv) + 1);
			after_args = tail;
		} else if (tk.str.eq({from_literal, "volatile"})) {
			cv = CvQual(unsigned(cv) + 2);
			after_args = tail;
		} else {
			break;
		}
	}

	// ref qualifiers
	Option<RefQual> ref_qual = none;
	{
		VEG_BIND(auto, (tk, tail), type_parse::parse_token(after_args));
		if (tk.str.eq({from_literal, "&"})) {
			ref_qual = some(RefQual::LVALUE);
			after_args = tail;
		} else if (tk.str.eq({from_literal, "&&"})) {
			ref_qual = some(RefQual::RVALUE);
			after_args = tail;
		}
	}

	// TODO
	// dependent expansions
	// [with ... = ... SEP ... = ...]
	// SEP is ; or ,
	// check ; first as it is less common (impossible?) in other contexts
	std::vector<Tuple<Entity, Entity>> dependent_expansions;

	return {
			VEG_FWD(name_ret)[1_c],
			VEG_FWD(name_ret)[0_c],
			VEG_FWD(args),
			VEG_FWD(cv),
			is_static,
			VEG_FWD(dependent_expansions),
	};
}
} // namespace type_parse

template <>
struct fmt::Debug<type_parse::FunctionDecl> {
	static void to_string(fmt::Buffer& out, Ref<type_parse::FunctionDecl> r) {
		out.append_literal("FunctionDecl ");
		fmt::DbgStructScope _{out};

		out.append_ln();
		out.append_literal("Function name: ");
		dbg_to(out, ref(r->full_name));

		out.append_ln();
		out.append_literal("Args: ");
		dbg_slice(
				out,
				Slice<type_parse::Entity>{
						from_raw_parts,
						r->args.data(),
						isize(r->args.size()),
						unsafe,
				});

		if (r->return_type.is_some()) {
			out.append_ln();
			out.append_literal("Return: ");
			dbg_to(out, r->return_type.as_ref().unwrap());
		}
	}
};
} // namespace veg

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
	void operator[](int) const { PRINT_FN(); }
	void operator()(int) const { PRINT_FN(); }
	void operator&&(A /*unused*/) const { PRINT_FN(); }
};
} // namespace test_cases

auto main() -> int {
	using namespace veg::type_parse;
	using namespace veg;

	dbg(parse_function_decl(
			{from_literal,
	     "::veg::internal::meta_::discard_1st< "
	     "::veg::meta::enable_if_t<((::veg::concepts::movable<T>) && "
	     "::veg::meta::bool_constant<(_ == 0)>::value)>, T> "
	     "veg::Option<int>::unwrap() && [T = int, _ = 0]"}));

	VEG_ASSERT(
			dbg(greedy_parse_entity(
					StrView{from_literal, "const int volatile * const & "})[0_c]) ==
			Entity{
					Entity::Uwunion{
							3_c,
							Pointer{
									box(Entity{
											Entity::Uwunion{
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
		constexpr StrView tests[] = {{from_literal, "void test_cases::plain_fn()"}};
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
							Entity::Uwunion{
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
							Entity::Uwunion{
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
							Entity::Uwunion{
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
		     "decltype (T()) test_cases::TplS<T>::plain_fn_trailing() const [with "
		     "T = void; decltype (T()) = void]"},
				{from_literal,
		     "<dependent type> test_cases::TplS<void>::plain_fn_trailing() const "
		     "[T = void]"}};

		{
			auto test = tests[0];
			VEG_ASSERT(
					greedy_parse_entity(test)[0_c] ==
					Entity{
							Entity::Uwunion{
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
							Entity::Uwunion{
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
		     "void test_cases::TplS<void>::tpl_fn_trailing() const [T = void, U = "
		     "(lambda at b.cpp:360:12)]"}};

		for (auto test : tests) {
			VEG_ASSERT(
					greedy_parse_nested_entity(test)[0_c].kind.unwrap(0_c).name ==
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
		     "test_cases::TplS<void>::DtorInner<int>::~DtorInner() [T = void, U =	"
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
