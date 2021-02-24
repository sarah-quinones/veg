import sys

MACRO_PREFIX = "__VEG"


def main():
    """
    merge boost preprocessor files
    """
    try:
        boost_dir = sys.argv[1]
    except IndexError as _:
        boost_dir = "/usr/local/include/"

    old_paths = [
        f"{boost_dir}/boost/preprocessor/stringize.hpp",
        f"{boost_dir}/boost/preprocessor/variadic/size.hpp",
        f"{boost_dir}/boost/preprocessor/tuple/enum.hpp",
        f"{boost_dir}/boost/preprocessor/seq/seq.hpp",
        f"{boost_dir}/boost/preprocessor/detail/auto_rec.hpp",
        f"{boost_dir}/boost/preprocessor/control/iif.hpp",
        f"{boost_dir}/boost/preprocessor/punctuation/comma.hpp",
        f"{boost_dir}/boost/preprocessor/list/adt.hpp",
        f"{boost_dir}/boost/preprocessor/punctuation/comma_if.hpp",
        f"{boost_dir}/boost/preprocessor/config/config.hpp",
        f"{boost_dir}/boost/preprocessor/facilities/identity.hpp",
        f"{boost_dir}/boost/preprocessor/punctuation/detail/is_begin_parens.hpp",
        f"{boost_dir}/boost/preprocessor/array/elem.hpp",
        f"{boost_dir}/boost/preprocessor/tuple/rem.hpp",
        f"{boost_dir}/boost/preprocessor/arithmetic/dec.hpp",
        f"{boost_dir}/boost/preprocessor/control/expr_iif.hpp",
        f"{boost_dir}/boost/preprocessor/punctuation/remove_parens.hpp",
        f"{boost_dir}/boost/preprocessor/control/detail/while.hpp",
        f"{boost_dir}/boost/preprocessor/variadic/to_seq.hpp",
        f"{boost_dir}/boost/preprocessor/logical/bitand.hpp",
        f"{boost_dir}/boost/preprocessor/comparison/greater.hpp",
        f"{boost_dir}/boost/preprocessor/comparison/less.hpp",
        f"{boost_dir}/boost/preprocessor/tuple/to_array.hpp",
        f"{boost_dir}/boost/preprocessor/facilities/overload.hpp",
        f"{boost_dir}/boost/preprocessor/repetition/deduce_z.hpp",
        f"{boost_dir}/boost/preprocessor/list/fold_right.hpp",
        f"{boost_dir}/boost/preprocessor/array/size.hpp",
        f"{boost_dir}/boost/preprocessor/cat.hpp",
        f"{boost_dir}/boost/preprocessor/tuple/to_seq.hpp",
        f"{boost_dir}/boost/preprocessor/array/data.hpp",
        f"{boost_dir}/boost/preprocessor/detail/check.hpp",
        f"{boost_dir}/boost/preprocessor/repetition/repeat.hpp",
        f"{boost_dir}/boost/preprocessor/tuple/eat.hpp",
        f"{boost_dir}/boost/preprocessor/list/detail/fold_right.hpp",
        f"{boost_dir}/boost/preprocessor/variadic/elem.hpp",
        f"{boost_dir}/boost/preprocessor/tuple/detail/is_single_return.hpp",
        f"{boost_dir}/boost/preprocessor/comparison/not_equal.hpp",
        f"{boost_dir}/boost/preprocessor/tuple/elem.hpp",
        f"{boost_dir}/boost/preprocessor/logical/bool.hpp",
        f"{boost_dir}/boost/preprocessor/logical/not.hpp",
        f"{boost_dir}/boost/preprocessor/list/detail/fold_left.hpp",
        f"{boost_dir}/boost/preprocessor/facilities/empty.hpp",
        f"{boost_dir}/boost/preprocessor/control/while.hpp",
        f"{boost_dir}/boost/preprocessor/list/fold_left.hpp",
        f"{boost_dir}/boost/preprocessor/logical/compl.hpp",
        f"{boost_dir}/boost/preprocessor/repetition/enum.hpp",
        f"{boost_dir}/boost/preprocessor/list/reverse.hpp",
        f"{boost_dir}/boost/preprocessor/detail/is_binary.hpp",
        f"{boost_dir}/boost/preprocessor/repetition/for.hpp",
        f"{boost_dir}/boost/preprocessor/seq/detail/is_empty.hpp",
        f"{boost_dir}/boost/preprocessor/seq/for_each_i.hpp",
        f"{boost_dir}/boost/preprocessor/repetition/detail/for.hpp",
        f"{boost_dir}/boost/preprocessor/control/if.hpp",
        f"{boost_dir}/boost/preprocessor/arithmetic/inc.hpp",
        f"{boost_dir}/boost/preprocessor/comparison/less_equal.hpp",
        f"{boost_dir}/boost/preprocessor/seq/elem.hpp",
        f"{boost_dir}/boost/preprocessor/facilities/expand.hpp",
        f"{boost_dir}/boost/preprocessor/tuple/size.hpp",
        f"{boost_dir}/boost/preprocessor/array/pop_front.hpp",
        f"{boost_dir}/boost/preprocessor/tuple/pop_front.hpp",
        f"{boost_dir}/boost/preprocessor/arithmetic/sub.hpp",
        f"{boost_dir}/boost/preprocessor/seq/for_each.hpp",
        f"{boost_dir}/boost/preprocessor/seq/size.hpp",
        f"{boost_dir}/boost/preprocessor/array/to_tuple.hpp",
        f"{boost_dir}/boost/preprocessor/punctuation/is_begin_parens.hpp",
        f"{boost_dir}/boost/preprocessor/debug/error.hpp",
    ]

    old_paths = list(set(old_paths))

    for i, name in enumerate(old_paths):
        if "config/config.hpp" in name:
            old_paths[i], old_paths[0] = old_paths[0], old_paths[i]
            break

    for name in old_paths:
        with open(name) as file:
            line: str = file.readline()
            while line:
                line = line.replace(
                    "BOOST_PREPROCESSOR", f"{MACRO_PREFIX}_BOOST_PREPROCESSOR"
                )
                line = line.replace("BOOST_PP_", f"{MACRO_PREFIX}_PP_")
                if "include" in line:
                    line = ""
                print(line, end="")
                line = file.readline()


if __name__ == "__main__":
    main()
