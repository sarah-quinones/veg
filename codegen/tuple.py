from sys import argv

B = "{"
E = "}"


def nth_tuple(n_elems):
    """
    codegen for aggregate tuple of N elements
    """
    comma_sep = ""
    t_param_decl = ""
    t_params = ""
    var_decl = ""
    for i in range(n_elems):
        t_param_decl += f"{comma_sep}typename T{i}"
        t_params += f"{comma_sep}T{i}"
        var_decl += f"T{i} _{i}; "
        comma_sep = ", "
    var_decl = var_decl.rstrip()
    tup_def = (
        f"template <{t_param_decl}> struct tuple<{t_params}> {B}"
        f"{var_decl}\n"
        + (
            (
                f"template <usize I, typename T> friend constexpr auto get(T&& arg) noexcept\n"
                f"-> VEG_REQUIRES_RET2((I < {n_elems}) && "
                f"meta::is_same_v<meta::remove_cvref_t<T>, tuple>,\n"
                f"decltype(internal::tuple::get_impl<I>::apply(VEG_FWD(arg))))\n"
                f"{B} return internal::tuple::get_impl<I>::apply(VEG_FWD(arg)); {E}\n"
                f"template <i64 I> constexpr auto operator[](fix<I> /*arg*/) & noexcept"
                f"-> VEG_REQUIRES_RET((I < {n_elems}) && (I >= 0),\n"
                f"decltype(internal::tuple::adl_get<I>(*this)))"
                f"{B} return internal::tuple::adl_get<I>(*this); {E}\n"
                f"template <i64 I> constexpr auto operator[](fix<I> /*arg*/) const& noexcept"
                f"-> VEG_REQUIRES_RET((I < {n_elems}) && (I >= 0),\n"
                f"decltype(internal::tuple::adl_get<I>(*this)))"
                f"{B} return internal::tuple::adl_get<I>(*this); {E}\n"
                f"template <i64 I> constexpr auto operator[](fix<I> /*arg*/) && noexcept"
                f"-> VEG_REQUIRES_RET((I < {n_elems}) && (I >= 0),\n"
                f"decltype(internal::tuple::adl_get<I>(static_cast<tuple&&>(*this))))"
                f"{B} return internal::tuple::adl_get<I>(static_cast<tuple&&>(*this)); {E}\n"
                f"template <i64 I> constexpr auto operator[](fix<I> /*arg*/) const&& noexcept"
                f"-> VEG_REQUIRES_RET((I < {n_elems}) && (I >= 0),\n"
                f"decltype(internal::tuple::adl_get<I>(static_cast<tuple const&&>(*this))))"
                f"{B} return internal::tuple::adl_get<I>(static_cast<tuple const&&>(*this)); {E}\n"
            )
            if n_elems > 0
            else ""
        )
        + f"{E};\n"
    )
    return tup_def


def nth_get(ith_elem):
    """
    codegen for adl get<I>(tuple)
    """
    return (
        f"template <> struct get_impl<{ith_elem}> {B}\n"
        f"template <typename T> static constexpr auto apply(T&& tup) noexcept"
        f"-> auto&& "
        f"{B} return (VEG_FWD(tup)._{ith_elem}); {E}\n"
        f"template <{', '.join([f'typename T{i}' for i in range(ith_elem+1)])}, typename... Ts>"
        f"using type = T{ith_elem}; "
        f"{E};"
    )


def main(n_upper):
    """
    generate tuple specialization file without header guards
    """
    print(f"#define VEG_TUPLE_SPECIALIZATION_COUNT {n_upper+1}")
    print('#include "veg/internal/type_traits.hpp"')
    print('#include "veg/internal/meta_int.hpp"')
    print("namespace veg {")
    print("// clang-format off")
    print("using usize = decltype(sizeof(0));")
    print("namespace internal {")
    print("namespace tuple {")
    print("template <typename T> auto get() = delete;")
    print(
        "template <usize I, typename T> auto adl_get(T&& arg)\n"
        "noexcept(noexcept(get<I>(VEG_FWD(arg))))\n"
        "-> decltype(get<I>(VEG_FWD(arg))) { return get<I>(VEG_FWD(arg)); }"
    )
    print("template <typename Ts> struct element;")
    print("template <decltype(sizeof(int)) I> struct get_impl;")
    print()

    for i in range(n_upper):
        print(nth_get(i))

    print()
    print("} // namespace tuple")
    print("} // namespace internal")
    print()
    print("template <typename... Ts> struct tuple;")

    for i in range(n_upper + 1):
        print(nth_tuple(i))

    print("// clang-format on")
    print("} // namespace veg")


if __name__ == "__main__":
    main(int(argv[1]))
