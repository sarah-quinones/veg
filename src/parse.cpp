#include <veg/internal/parse.hpp>
#include <cstdio>
#include <string>

namespace veg {
namespace internal {
namespace type_parse {

auto is_ident_char(char c) noexcept -> bool {
	return (c == '~') ||             //
	       (c == '_') ||             //
	       (c >= 'a' && c <= 'z') || //
	       (c >= 'A' && c <= 'Z') || //
	       (c >= '0' && c <= '9');
}

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
						tuplify,
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
				tuplify,
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
	return {tuplify, {tk, TokenKind::IDENT}, tail};
}

auto parse_entity(StrView str) -> Entity {
	return type_parse::greedy_parse_entity(str)[0_c];
}

auto parse_templated_entity(StrView between_angled_brackets) -> Vec<Entity> {
	Vec<Entity> out;
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

		out.push(type_parse::parse_entity(head));
		if (tail.len() == 0) {
			// no more commas
			// this was the last param
			break;
		}
		between_angled_brackets = tail.skip_leading(1);
	}
	return out;
}

auto greedy_parse_templated_entity(StrView after_angled, StrView end_delim)
		-> Tuple<Vec<Entity>, StrView> {
	VEG_BIND( //
			auto,
			(between_angled, tail),
			type_parse::find_matching(end_delim, after_angled));
	assert(tail.begins_with(end_delim));
	return {
			tuplify,
			type_parse::parse_templated_entity(between_angled),
			tail.skip_leading(1),
	};
}

auto greedy_parse_nestable_entity(StrView str) noexcept
		-> Tuple<Entity, StrView> {
	VEG_BIND(auto, (tk, tail), type_parse::parse_token(str));

	if (tk.str == StrView({from_literal, "<"}) ||
	    tk.str == StrView({from_literal, "{"})) {
		tail = str;
		tk = empty_tk;
	}
	if (tail.begins_with({from_literal, "<"}) ||
	    tail.begins_with({from_literal, "{"})) {

		Entity tpl_head = Entity{
				Entity::Uwunion{
						Tag<EntityName>{},
						{tk.str},
				},
				CvQual::NONE,
		};

		VEG_BIND(
				auto,
				(tpl_args, tail2),
				type_parse::greedy_parse_templated_entity(
						tail.skip_leading(1),
						{from_literal, tail.begins_with({from_literal, "{"}) ? "}" : ">"}));

		return Tuple<Entity, StrView>{
				tuplify,
				Entity{
						Entity::Uwunion{
								Tag<TemplatedEntity>{},
								{
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
			tuplify,
			Entity{
					Entity::Uwunion{
							Tag<EntityName>{},
							{tk.str},
					},
					CvQual::NONE,
			},
			tail};
}

auto greedy_parse_nested_entity(StrView str) noexcept
		-> Tuple<Entity, StrView> {
	// skip global scope
	{
		VEG_BIND(auto, (tk, after_tk), type_parse::parse_token(str));
		if (tk.str.eq({from_literal, "::"})) {
			str = after_tk;
		}
	}

	Vec<Entity> v;
	while (true) {
		VEG_BIND(auto, (e, tail), type_parse::greedy_parse_nestable_entity(str));
		VEG_BIND(auto, (next_tk, after_token), type_parse::parse_token(tail));

		str = tail;
		v.push(VEG_FWD(e));
		if (next_tk.str.eq({from_literal, "::"})) {
			str = after_token;
		} else {
			break;
		}
	}
	if (v.len() == 1) {
		return {
				tuplify,
				static_cast<Entity&&>(v[0]),
				str,
		};
	}

	return {
			tuplify,
			Entity{
					Entity::Uwunion{
							Tag<NestedEntity>{},
							{VEG_FWD(v)},
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

auto greedy_parse_entity(StrView str) noexcept -> Tuple<Entity, StrView> {
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
					tuplify,
					Entity{
							Entity::Uwunion{
									Tag<EntityName>{},
									{inside_parens},
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
							Tag<Pointer>{},
							{box(VEG_FWD(e)), RefQual::POINTER},
					},
					CvQual::NONE,
			};
			tail = tail2;
		} else if (tk.str.eq({from_literal, "&&"})) {
			e = Entity{
					Entity::Uwunion{
							Tag<Pointer>{},
							{box(VEG_FWD(e)), RefQual::RVALUE},
					},
					CvQual::NONE,
			};
			tail = tail2;
		} else if (tk.str.eq({from_literal, "&"})) {
			e = Entity{
					Entity::Uwunion{
							Tag<Pointer>{},
							{box(VEG_FWD(e)), RefQual::LVALUE},
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
							Tag<Array>{},
							{
									box(VEG_FWD(e)),
									box(Entity{
											Entity::Uwunion{Tag<EntityName>{}, {size_str}},
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
			tuplify,
			VEG_FWD(e),
			tail,
	};
}

auto parse_function_decl(StrView str) noexcept -> FunctionDecl {
	bool is_static = false;

	// scan for static and constexpr
	while (true) {
		VEG_BIND(auto, (tk, after_static), type_parse::parse_token(str));
		if (tk.str.eq({from_literal, "static"})) {
			is_static = true;
			str = after_static;
		} else if (tk.str.eq({from_literal, "constexpr"})) {
			str = after_static;
		} else {
			break;
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
					tuplify,
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
					tuplify,
					VEG_FWD(actual_name),
					some(VEG_FWD(fn_name_or_return)),
			};
		}
	}();

	assert(after_name.begins_with({from_literal, "("}));
	after_name = after_name.skip_leading(1);

	VEG_BIND( //
			auto,
			(between_paren, after_args),
			type_parse::find_matching({from_literal, ")"}, after_name));
	auto args = type_parse::parse_templated_entity(between_paren);

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

	Vec<Tuple<Entity, Entity>> dependent_expansions;
	{
		VEG_BIND(auto, (tk, tail), type_parse::parse_token(after_args));
		bool has_with = tail.begins_with({from_literal, "with"});

		if (tk.str.eq({from_literal, "["})) {
			after_args = tail.skip_leading(has_with ? 4 : 0);

			VEG_BIND( //
					auto,
					(between_square, after_params),
					type_parse::find_matching({from_literal, "]"}, after_args));
			while (true) {
				between_square = between_square.trim(' ');
				if (between_square.len() == 0) {
					break;
				}

				VEG_BIND(
						auto,
						(head, tail),
						type_parse::find_matching({from_literal, "="}, between_square));
				auto lhs = type_parse::parse_entity(head);
				VEG_BIND(
						auto,
						(tail_head, tail_tail),
						type_parse::find_matching(
								{from_literal, has_with ? ";" : ","}, tail.skip_leading(1)));

				auto rhs = type_parse::parse_entity(tail_head.trim(' '));
				auto expansion = tuplify(VEG_FWD(lhs), VEG_FWD(rhs));
				dependent_expansions.push(VEG_FWD(expansion));
				between_square = tail_tail.skip_leading((tail_tail.len() == 0) ? 0 : 1);
			}
		}
	}

	return {
			VEG_FWD(name_ret)[1_c],
			VEG_FWD(name_ret)[0_c],
			VEG_FWD(args),
			VEG_FWD(cv),
			is_static,
			VEG_FWD(dependent_expansions),
	};
}

void strip_discard_1st(RefMut<Entity> e_mut) noexcept {
	using Uwunion = Entity::Uwunion;
	auto nested_t = Tag<NestedEntity>{};

	auto e = e_mut.as_const();

	if (!e->kind.holds(nested_t)) {
		return;
	}
	auto const& nested = e->kind[nested_t];
	if (nested.components.len() != 4) {
		return;
	}

	auto name_t = Tag<EntityName>{};
	auto elems = nested.components.as_ref();
	if (!(elems[0] == Entity{Uwunion{name_t, {{from_literal, "veg"}}}} &&
	      elems[1] == Entity{Uwunion{name_t, {{from_literal, "internal"}}}} &&
	      elems[2] == Entity{Uwunion{name_t, {{from_literal, "meta_"}}}})) {
		return;
	}

	auto const& last = elems[3];
	auto tpl_t = Tag<TemplatedEntity>{};
	if (!last.kind.holds(tpl_t)) {
		return;
	}
	auto const& as_tpl = last.kind[tpl_t];
	if (!(*as_tpl.tpl.ptr() ==
	      Entity{Uwunion{name_t, {{from_literal, "discard_1st"}}}})) {
		return;
	}
	if (as_tpl.args.len() != 2) {
		return;
	}
	*e_mut = as_tpl.args[1];
}

void recurse_strip_discard_1st(RefMut<Entity> e_mut) noexcept {
	type_parse::strip_discard_1st(VEG_FWD(e_mut));
	e_mut->kind.as_mut().visit_i(tuplify(
			[](RefMut<EntityName> e) noexcept {},
			[](RefMut<TemplatedEntity> e) noexcept {
				type_parse::recurse_strip_discard_1st(mut(*e->tpl));
				for (isize i = 0; i < e->args.len(); ++i) {
					type_parse::recurse_strip_discard_1st(mut(e->args[i]));
				}
			},
			[](RefMut<NestedEntity> e) noexcept {
				for (isize i = 0; i < e->components.len(); ++i) {
					type_parse::recurse_strip_discard_1st(mut(e->components[i]));
				}
			},
			[](RefMut<Pointer> e) noexcept {
				type_parse::recurse_strip_discard_1st(mut(*e->entity));
			},
			[](RefMut<Array> e) noexcept {
				type_parse::recurse_strip_discard_1st(mut(*e->entity));
				type_parse::recurse_strip_discard_1st(mut(*e->size));
			},
			[](RefMut<Function> e) noexcept {
				type_parse::recurse_strip_discard_1st(mut(*e->return_type));
				for (isize i = 0; i < e->args.len(); ++i) {
					type_parse::recurse_strip_discard_1st(mut(e->args[i]));
				}
			})

	);
}

template <typename Out>
void print_sv(RefMut<Out> out, StrView str) noexcept {
	out->append(str.ptr(), str.len());
}
template <typename Out>
void print_cv(RefMut<Out> out, CvQual cv) noexcept {
	switch (cv) {
	case CvQual::NONE:
		break;
	case CvQual::CONST:
		type_parse::print_sv(VEG_FWD(out), {from_literal, "const "});
		break;
	case CvQual::VOLATILE:
		type_parse::print_sv(VEG_FWD(out), {from_literal, "volatile "});
		break;
	case CvQual::CONST_VOLATILE:
		type_parse::print_sv(VEG_FWD(out), {from_literal, "const volatile "});
		break;
	default:
		break;
	}
}
void print_entity_to(std::FILE* f, Ref<Entity> e, isize indent) noexcept;

template <typename Fn, typename T, typename Out>
void print_slice(RefMut<Out> out, Fn fn, Slice<T> es, isize indent) {
	auto new_indent = indent;
	bool newline = false;
	if (es.len() > 1) {
		++new_indent;
		newline = true;
	}
	for (isize i = 0; i < es.len(); ++i) {
		if (newline) {
			out->append_c('\n');
			out->append_n('\t', new_indent);
		}
		fn(VEG_FWD(out), ref(es[i]), new_indent);
	}

	if (newline) {
		out->append_c('\n');
		out->append_n('\t', indent);
	}
}

template <typename Out>
void print_entity_to(RefMut<Out> out, Ref<Entity> e, isize indent) noexcept {
	type_parse::print_cv(VEG_FWD(out), e->cv_qual);

	e->kind.as_ref().visit_i(tuplify(
			[&](Ref<EntityName> e) noexcept {
				type_parse::print_sv(VEG_FWD(out), e->name);
			},
			[&](Ref<TemplatedEntity> e) noexcept {
				type_parse::print_entity_to(VEG_FWD(out), ref(*e->tpl), indent);
				out->append_c('<');
				type_parse::print_slice(
						VEG_FWD(out), print_entity_to<Out>, e->args.as_ref(), indent);
				out->append_c('>');
			},
			[&](Ref<NestedEntity> e) noexcept {
				bool print_sep = false;
				for (isize i = 0; i < e->components.len(); ++i) {
					if (print_sep) {
						out->append_n(':', 2);
					}
					type_parse::print_entity_to(
							VEG_FWD(out), ref(e->components[i]), indent);
					print_sep = true;
				}
			},
			[&](Ref<Pointer> e) noexcept {
				switch (e->ref_qual) {
				case RefQual::POINTER:
					out->append_c('*');
					break;
				case RefQual::LVALUE:
					out->append_c('&');
					break;
				case RefQual::RVALUE:
					out->append_n('&', 2);
					break;
				default:
					break;
				}
				type_parse::print_entity_to(VEG_FWD(out), ref(*e->entity), indent);
			},
			[&](Ref<Array> e) noexcept {
				out->append_c('[');
				type_parse::print_entity_to(VEG_FWD(out), ref(*e->entity), indent);
				out->append_c(';');
				out->append_c(' ');
				type_parse::print_entity_to(VEG_FWD(out), ref(*e->size), indent);
				out->append_c(']');
			},
			[&](Ref<Function> e) noexcept {
				type_parse::print_sv(VEG_FWD(out), {from_literal, "fn ("});
				type_parse::print_slice(
						VEG_FWD(out), print_entity_to<Out>, e->args.as_ref(), indent);
				type_parse::print_sv(VEG_FWD(out), {from_literal, ") -> "});
				type_parse::print_entity_to(VEG_FWD(out), ref(*e->return_type), indent);
			}));
}

struct Void {};

template <typename Out>
void print_decl_to(RefMut<Out> out, FunctionDecl decl) noexcept {
	if (decl.is_static) {
		type_parse::print_sv(VEG_FWD(out), {from_literal, "static "});
	}
	print_cv(VEG_FWD(out), decl.cv_qual);
	type_parse::recurse_strip_discard_1st(decl.return_type.as_mut().unwrap());
	for (isize i = 0; i < decl.args.len(); ++i) {
		type_parse::recurse_strip_discard_1st(mut(decl.args[i]));
	}

	type_parse::print_sv(VEG_FWD(out), {from_literal, "fn "});
	type_parse::print_entity_to(VEG_FWD(out), ref(decl.full_name), 0);
	out->append_c('(');
	type_parse::print_slice(
			VEG_FWD(out), print_entity_to<Out>, decl.args.as_ref(), 0);
	out->append_c(')');

	decl.return_type.as_ref().map_or( //
			[&](Ref<Entity> ret) noexcept -> Void {
				type_parse::print_sv(VEG_FWD(out), {from_literal, "\n\nreturns: "});
				type_parse::print_entity_to(VEG_FWD(out), ret, 0);
				return {};
			},
			{});

	if (decl.dependent_expansions.len() == 0) {
		return;
	}

	type_parse::print_sv(VEG_FWD(out), {from_literal, "\n\nwith: "});
	type_parse::print_slice(
			VEG_FWD(out),
			[](RefMut<Out> out,
	       Ref<Tuple<Entity, Entity>> tup,
	       isize indent) noexcept {
				type_parse::print_entity_to(VEG_FWD(out), tup->as_ref()[0_c], indent);
				type_parse::print_sv(VEG_FWD(out), {from_literal, " = "});
				type_parse::print_entity_to(VEG_FWD(out), tup->as_ref()[1_c], indent);
			},
			decl.dependent_expansions.as_ref(),
			0);
}

void function_decl_to_str(RefMut<std::string> str, FunctionDecl decl) noexcept {
	struct StringWrapper {
		std::string _;
		void append_c(char c) noexcept { _ += c; }
		void append(char const* s, usize n) noexcept { _.append(s, n); }
		void append_n(char c, usize n) noexcept { _.append(n, c); }
	};
	auto tmp = StringWrapper{static_cast<std::string&&>(*str)};
	type_parse::print_decl_to(mut(tmp), VEG_FWD(decl));
	*str = VEG_FWD(tmp._);
}
void function_decl_to_file(std::FILE* f, FunctionDecl decl) noexcept {
	struct FileWrapper {
		std::FILE* _;
		void append_c /* NOLINT */ (char c) noexcept { std::fputc(c, _); }
		void append /* NOLINT */ (char const* s, isize n) noexcept {
			std::fwrite(s, 1, n, _);
		}
		void append_n /* NOLINT */ (char c, isize n) noexcept {
			for (isize i = 0; i < n; ++i) {
				append_c(c);
			}
		}
	};
	auto tmp = FileWrapper{f};
	type_parse::print_decl_to(mut(tmp), VEG_FWD(decl));
}
} // namespace type_parse
} // namespace internal
} // namespace veg
