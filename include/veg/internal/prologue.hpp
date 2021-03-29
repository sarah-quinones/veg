#ifdef VEG_PROLOGUE
#error "missing epilogue"
#endif
#define VEG_PROLOGUE

#define VEG_DECLVAL(...) (static_cast<__VA_ARGS__ (*)()>(nullptr)())

#if __cplusplus >= 201703L
#define VEG_DECLVAL_NOEXCEPT(...)                                              \
	(static_cast<__VA_ARGS__ (*)() noexcept>(nullptr)())
#else
#define VEG_DECLVAL_NOEXCEPT(...)                                              \
	(::veg::internal::meta_::declval<__VA_ARGS__>())
#endif

#define VEG_DEDUCE_RET(...)                                                    \
	noexcept(noexcept(__VA_ARGS__))->decltype(__VA_ARGS__) {                     \
		return __VA_ARGS__;                                                        \
	}                                                                            \
	VEG_NOM_SEMICOLON

#define VEG_IMPL_AND(I, _, Param) &&__VEG_PP_UNWRAP(Param)
#define VEG_IMPL_OR(I, _, Param) || __VEG_PP_UNWRAP(Param)

#if __cplusplus >= 201703L
#define VEG_ALL_OF(...) (__VA_ARGS__ && ... && true)
#define VEG_ANY_OF(...) (__VA_ARGS__ || ... || false)
#else
#define VEG_ALL_OF(...) ::veg::meta::all_of({(__VA_ARGS__)...})
#define VEG_ANY_OF(...) ::veg::meta::any_of({(__VA_ARGS__)...})
#endif

#if __cplusplus >= 202002L

#define VEG_IMPL_CONJUNCTION(First, ...)                                       \
	(__VEG_PP_UNWRAP(First)                                                      \
	     __VEG_PP_TUPLE_FOR_EACH_I(VEG_IMPL_AND, _, (__VA_ARGS__)))

#define VEG_IMPL_DISJUNCTION(First, ...)                                       \
	(__VEG_PP_UNWRAP(First)                                                      \
	     __VEG_PP_TUPLE_FOR_EACH_I(VEG_IMPL_OR, _, (__VA_ARGS__)))

#define VEG_DEF_CONCEPT(Tpl, Name, ...)                                        \
	template <__VEG_PP_REMOVE_PAREN1(Tpl)>                                       \
	concept Name = __VA_ARGS__;

#define VEG_CHECK_CONCEPT(...)                                                 \
	static_assert(                                                               \
			::veg::concepts::__VA_ARGS__, __VEG_PP_STRINGIZE(__VA_ARGS__) " failed")
#define VEG_CONCEPT(...) ::veg::concepts::__VA_ARGS__
#define VEG_DEF_CONCEPT_CONJUNCTION(Tpl, Name, Terms)                          \
	VEG_DEF_CONCEPT(Tpl, Name, VEG_IMPL_CONJUNCTION Terms)
#define VEG_DEF_CONCEPT_DISJUNCTION(Tpl, Name, Terms)                          \
	VEG_DEF_CONCEPT(Tpl, Name, VEG_IMPL_DISJUNCTION Terms)

#else

#if __cplusplus >= 201703L
#define VEG_IMPL_DEF_CONCEPT(Tpl, Name, Value, ...)                            \
	namespace _ {                                                                \
	template <__VEG_PP_REMOVE_PAREN1(Tpl)>                                       \
	struct Name : __VA_ARGS__ {};                                                \
	}                                                                            \
	template <__VEG_PP_REMOVE_PAREN1(Tpl)>                                       \
	inline constexpr bool const& Name = Value
#elif __cplusplus >= 201402L
#define VEG_IMPL_DEF_CONCEPT(Tpl, Name, Value, ...)                            \
	namespace _ {                                                                \
	template <__VEG_PP_REMOVE_PAREN1(Tpl)>                                       \
	struct Name : __VA_ARGS__ {};                                                \
	}                                                                            \
	namespace {                                                                  \
	VEG_IGNORE_CPP14_EXTENSION_WARNING(                                          \
			template <__VEG_PP_REMOVE_PAREN1(Tpl)>                                   \
			constexpr bool const& Name = ::veg::meta::bool_constant<Value>::value);  \
	}                                                                            \
	VEG_NOM_SEMICOLON
#else
#define VEG_IMPL_DEF_CONCEPT(Tpl, Name, Value, ...)                            \
	template <__VEG_PP_REMOVE_PAREN1(Tpl)>                                       \
	struct Name : __VA_ARGS__ {}
#endif

#if __cplusplus >= 201402L
#define VEG_CONCEPT(...) ::veg::concepts::__VA_ARGS__
#define VEG_IMPL_ADD_VALUE(I, _, Param) (Param)
#define VEG_IMPL_TRAIT(Param) __VEG_PP_HEAD Param _::__VEG_PP_TAIL Param
#else
#define VEG_CONCEPT(...) ::veg::concepts::__VA_ARGS__::value
#define VEG_IMPL_ADD_VALUE(I, _, Param) ((__VEG_PP_REMOVE_PAREN(Param)::value))
#define VEG_IMPL_TRAIT(Param) __VEG_PP_UNWRAP(Param)
#endif
#define VEG_IMPL_PUT_TRAIT(I, _, Param) , VEG_IMPL_TRAIT(Param)

#define VEG_CHECK_CONCEPT(...)                                                 \
	static_assert(                                                               \
			decltype(::veg::concepts::check_##__VA_ARGS__())::value,                 \
			__VEG_PP_STRINGIZE(__VA_ARGS__) " failed")
#define VEG_DEF_CONCEPT(Tpl, Name, ...)                                        \
	VEG_IMPL_DEF_CONCEPT(                                                        \
			Tpl, Name, (__VA_ARGS__), ::veg::meta::bool_constant<__VA_ARGS__>);      \
	VEG_TEMPLATE(                                                                \
			Tpl, requires(__VA_ARGS__), , constexpr auto check_##Name, (_ = 0, int)) \
	noexcept->::veg::meta::true_type

#define VEG_IMPL_SFINAE(I, _, Param)                                           \
	, ::veg::meta::enable_if_t<__VEG_PP_UNWRAP Param, int> = 0

#define VEG_IMPL_OVERLOAD(I, Name_Tpl, Param)                                  \
	template <                                                                   \
			__VEG_PP_REMOVE_PAREN(__VEG_PP_TAIL Name_Tpl),                           \
			::veg::meta::enable_if_t<__VEG_PP_UNWRAP Param, int> = 0>                \
	void __VEG_PP_CAT(check_, __VEG_PP_HEAD Name_Tpl)() noexcept;

#define VEG_DEF_CONCEPT_BOOL_CONJUNCTION_IMPL(Tpl, Name, Base, Seq)            \
	VEG_IMPL_DEF_CONCEPT(                                                        \
			Tpl,                                                                     \
			Name,                                                                    \
			(__VEG_PP_REMOVE_PAREN1(Base)::value),                                   \
			__VEG_PP_REMOVE_PAREN1(Base));                                           \
	template <__VEG_PP_REMOVE_PAREN(Tpl)                                         \
	              __VEG_PP_TUPLE_FOR_EACH_I(VEG_IMPL_SFINAE, _, Seq)>            \
	void check_##Name() noexcept
#define VEG_DEF_CONCEPT_BOOL_DISJUNCTION_IMPL(Tpl, Name, Base, Seq)            \
	VEG_IMPL_DEF_CONCEPT(                                                        \
			Tpl,                                                                     \
			Name,                                                                    \
			(__VEG_PP_REMOVE_PAREN1(Base)::value),                                   \
			__VEG_PP_REMOVE_PAREN1(Base));                                           \
	__VEG_PP_TUPLE_FOR_EACH_I(VEG_IMPL_OVERLOAD, (Name, Tpl), Seq)               \
	VEG_NOM_SEMICOLON

#define VEG_DEF_CONCEPT_CONJUNCTION(Tpl, Name, Terms)                          \
	VEG_DEF_CONCEPT_BOOL_CONJUNCTION_IMPL(                                       \
			Tpl,                                                                     \
			Name,                                                                    \
			(VEG_IMPL_CONJUNCTION Terms),                                            \
			__VEG_PP_TUPLE_TRANSFORM_I(VEG_IMPL_ADD_VALUE, _, Terms))

#define VEG_DEF_CONCEPT_DISJUNCTION(Tpl, Name, Terms)                          \
	VEG_DEF_CONCEPT_BOOL_DISJUNCTION_IMPL(                                       \
			Tpl,                                                                     \
			Name,                                                                    \
			(VEG_IMPL_DISJUNCTION Terms),                                            \
			__VEG_PP_TUPLE_TRANSFORM_I(VEG_IMPL_ADD_VALUE, _, Terms))

#define VEG_IMPL_CONJUNCTION(First, ...)                                       \
	conjunction<VEG_IMPL_TRAIT(First) __VEG_PP_TUPLE_FOR_EACH_I(                 \
			VEG_IMPL_PUT_TRAIT, _, (__VA_ARGS__))>
#define VEG_IMPL_DISJUNCTION(First, ...)                                       \
	disjunction<VEG_IMPL_TRAIT(First) __VEG_PP_TUPLE_FOR_EACH_I(                 \
			VEG_IMPL_PUT_TRAIT, _, (__VA_ARGS__))>

#endif

#define VEG_IMPL_PARAM_EXPAND(I, _, Param)                                     \
	, __VEG_PP_TAIL Param __VEG_PP_HEAD Param
#define _VEG_IMPL_PARAM_EXPAND(I, _, Param)                                    \
	__VEG_PP_TAIL Param __VEG_PP_HEAD Param

#define VEG_TEMPLATE(TParams, Constraint, Attr, Name, ...)                     \
	VEG_IMPL_TEMPLATE(Attr, Name, TParams, Constraint, __VA_ARGS__)

#if __cplusplus >= 202002L
#define VEG_TEMPLATE_EXPLICIT(                                                 \
		Explicit_Cond, TParams, Constraint, Attr, Name, Params, ...)               \
	VEG_TEMPLATE(                                                                \
			TParams,                                                                 \
			Constraint,                                                              \
			explicit(Explicit_Cond) Attr,                                            \
			Name,                                                                    \
			__VEG_PP_REMOVE_PAREN(Params))                                           \
	__VA_ARGS__
#else
#define VEG_TEMPLATE_EXPLICIT(                                                 \
		Explicit_Cond, TParams, Constraint, Attr, Name, Params, ...)               \
	VEG_TEMPLATE(                                                                \
			(__VEG_PP_REMOVE_PAREN TParams,                                          \
	     ::veg::meta::enable_if_t<(Explicit_Cond), int> = 0),                    \
			Constraint,                                                              \
			explicit Attr,                                                           \
			Name,                                                                    \
			__VEG_PP_REMOVE_PAREN(Params))                                           \
	__VA_ARGS__                                                                  \
	VEG_TEMPLATE(                                                                \
			(__VEG_PP_REMOVE_PAREN TParams,                                          \
	     ::veg::meta::enable_if_t<!(Explicit_Cond), unsigned> = 0),              \
			Constraint,                                                              \
			Attr,                                                                    \
			Name,                                                                    \
			__VEG_PP_REMOVE_PAREN(Params))                                           \
	__VA_ARGS__
#endif

#define VEG_IMPL_TEMPLATE(Attr, Name, TParams, Constraint, ...)                \
	VEG_IMPL_TEMPLATE2(                                                          \
			Attr,                                                                    \
			Name,                                                                    \
			TParams,                                                                 \
			__VEG_PP_CAT(VEG_IMPL_PREFIX_, Constraint),                              \
			__VA_ARGS__)

#define VEG_IMPL_PREFIX_requires

#if __cplusplus >= 202002L
#define VEG_IMPL_TEMPLATE2(Attr, Name, TParams, Constraint, ...)               \
	template <__VEG_PP_REMOVE_PAREN(TParams)>                                    \
	requires Constraint Attr Name __VEG_PP_TUPLE_TRANSFORM_I(                    \
			_VEG_IMPL_PARAM_EXPAND, _, (__VA_ARGS__))
#else

#define VEG_IMPL_TEMPLATE2_HELPER_0(Constraint, Param)                         \
	__VEG_PP_TAIL Param __VEG_PP_HEAD Param

#define VEG_IMPL_TEMPLATE2_HELPER_1(Constraint, Param)                         \
	::veg::internal::meta_::                                                     \
			discard_1st<::veg::meta::enable_if_t<(Constraint)>, __VEG_PP_TAIL Param> \
					__VEG_PP_HEAD Param

#define VEG_IMPL_TEMPLATE2_HELPER(I, Constraint, Param)                        \
	__VEG_PP_CAT2(                                                               \
			VEG_IMPL_TEMPLATE2_HELPER_, __VEG_IMPL_PP_IS_1(__VEG_IMPL_PP_INC(I)))    \
	(Constraint, Param)

#define VEG_IMPL_TEMPLATE2(Attr, Name, TParams, Constraint, ...)               \
	template <__VEG_PP_REMOVE_PAREN(TParams)>                                    \
	Attr Name __VEG_PP_TUPLE_TRANSFORM_I(                                        \
			VEG_IMPL_TEMPLATE2_HELPER, Constraint, (__VA_ARGS__))

#endif

#define VEG_HAS_BUILTIN_OR_0(True, False) __VEG_PP_REMOVE_PAREN(False)
#define VEG_HAS_BUILTIN_OR_1(True, False) __VEG_PP_REMOVE_PAREN(True)
#define VEG_HAS_BUILTIN_OR(Builtin, True, False)                               \
	__VEG_PP_CAT(VEG_HAS_BUILTIN_OR_, VEG_HAS_BUILTIN(Builtin))(True, False)
#define VEG_DEF_CONCEPT_FROM_BUILTIN_OR_TRAIT(Tpl, Trait, Std_Trait, ...)      \
	VEG_DEF_CONCEPT(                                                             \
			Tpl,                                                                     \
			Trait,                                                                   \
			VEG_HAS_BUILTIN_OR(                                                      \
					__VEG_PP_CAT(__, Std_Trait),                                         \
					__VEG_PP_CAT(__, Std_Trait)(__VA_ARGS__),                            \
					(::std::Std_Trait<__VA_ARGS__>::value)))
#define VEG_DEF_CONCEPT_FROM_BUILTIN_OR_STD(Tpl, Trait, ...)                   \
	VEG_DEF_CONCEPT_FROM_BUILTIN_OR_TRAIT(                                       \
			Tpl, Trait, __VEG_PP_CAT(is_, Trait), __VA_ARGS__)

#define VEG_DEFAULT_CTOR_ASSIGN(Class)                                         \
	~Class() = default; /**/                                                     \
	Class(Class&&) = default;                                                    \
	Class(Class const&) = default;                                               \
	auto operator=(Class&&)&->Class& = default;                                  \
	auto operator=(Class const&)&->Class& = default

#if __cplusplus >= 201402L
#define VEG_CPP14(...) __VA_ARGS__
#else
#define VEG_CPP14(...)
#endif

#if __cplusplus >= 201703L
#define VEG_CPP17(...) __VA_ARGS__
#else
#define VEG_CPP17(...)
#endif

#if __cplusplus >= 202002L
#define VEG_CPP20(...) __VA_ARGS__
#else
#define VEG_CPP20(...)
#endif

#if defined(__has_builtin)
#define VEG_HAS_BUILTIN(x) __has_builtin(x)
#else
#define VEG_HAS_BUILTIN(x) 0
#endif

#if __cplusplus >= 201703L
#define VEG_NODISCARD [[nodiscard]]
#elif defined(__clang__)
#define VEG_NODISCARD HEDLEY_WARN_UNUSED_RESULT
#else
#define VEG_NODISCARD
#endif

#ifdef VEG_INTERNAL_ASSERTIONS
#define VEG_INTERNAL_ASSERT(...)                                               \
	VEG_ASSERT_ELSE("inner assertion failed", __VA_ARGS__)
#else
#define VEG_INTERNAL_ASSERT(...)                                               \
	VEG_DEBUG_ASSERT_ELSE("inner assertion failed", __VA_ARGS__)
#endif

#if __cplusplus >= 201402L

#define VEG_IGNORE_CPP14_EXTENSION_WARNING(...) __VA_ARGS__
#else
#define VEG_IGNORE_CPP14_EXTENSION_WARNING(...)
#endif

#define VEG_NOM_SEMICOLON static_assert(true, ".")

#if __cplusplus >= 202002L
#define VEG_ABI _20
#elif __cplusplus >= 201703L
#define VEG_ABI _17
#elif __cplusplus >= 201402L
#define VEG_ABI _14
#elif __cplusplus >= 201102L
#define VEG_ABI _11
#else
#error "[veg] c++ standards earlier than c++11 are not supported"
#endif

#define VEG_ABI_VERSION v0

#ifdef VEG_MODE_DOCS
#define VEG_DOC(...) __VA_ARGS__
#define VEG_DOC_LOCATION char loc[__LINE__] = __FILE__
#else
#define VEG_DOC(...)
#define VEG_DOC_LOCATION VEG_NOM_SEMICOLON
#endif
#define VEG_DOC_FN VEG_DOC_LOCATION;    /* veg::@func */
#define VEG_DOC_CTOR VEG_DOC_LOCATION;  /* veg::@ctor */
#define VEG_DOC_CLASS VEG_DOC_LOCATION; /* veg::@class */
