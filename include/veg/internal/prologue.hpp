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
