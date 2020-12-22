#ifndef VEG_STD_HPP_NS3C5ON6S
#define VEG_STD_HPP_NS3C5ON6S

#ifdef __has_include
#define VEG_HAS_INCLUDE(x) __has_include(x)
#else
#define VEG_HAS_INCLUDE(x) 0
#endif

#include <cstddef>          // maybe unneeded
#include <cstdint>          // i64, u64
#include <initializer_list> // std::initializer_list
#include <new>              // placement new, launder

// clang-format off
// construct_at, destroy_at, addressof
#if VEG_HAS_INCLUDE(<bits/stl_construct.h>) && VEG_HAS_INCLUDE(<bits/move.h>)
#include <bits/stl_construct.h>
#include <bits/move.h>
#else
#include <memory>
#endif
// clang-format on

#include <type_traits> // lots
#include <iosfwd>      // basic_ostream

#endif /* end of include guard VEG_STD_HPP_NS3C5ON6S */
