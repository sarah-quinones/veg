#ifndef __VEG_STD_HPP_NS3C5ON6S
#define __VEG_STD_HPP_NS3C5ON6S

#ifdef __has_include
#define __VEG_HAS_INCLUDE(x) __has_include(x)
#else
#define __VEG_HAS_INCLUDE(x) 0
#endif

#include <cstddef>          // maybe unneeded
#include <cstdint>          // i64, u64
#include <initializer_list> // std::initializer_list
#include <new>              // placement new, launder

// clang-format off
// construct_at, destroy_at, addressof
#if __VEG_HAS_INCLUDE(<bits/stl_construct.h>) && __VEG_HAS_INCLUDE(<bits/move.h>) && __VEG_HAS_INCLUDE(<bits/stl_iterator_base_types.h>) && __VEG_HAS_INCLUDE(<bits/stl_iterator_base_funcs.h>)
#include <bits/stl_iterator_base_types.h>
#include <bits/stl_iterator_base_funcs.h>
#include <bits/stl_construct.h>
#include <bits/move.h>
#else
#include <memory>
#endif
// clang-format on

#include <type_traits> // lots
#include <iosfwd>      // basic_ostream

#endif /* end of include guard __VEG_STD_HPP_NS3C5ON6S */
