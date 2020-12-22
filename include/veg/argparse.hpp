/**
 * Copyright (c) 2020 sarah k.
 * Copyright (C) 2012-2015 Yecheng Fu <cofyc.jackson at gmail dot com>
 * All rights reserved.
 *
 * Use of this source code is governed by a MIT-style license that can be found
 * in the LICENSE file.
 */

#ifndef VEG_ARGPARSE_HPP_CQUZWBHFS
#define VEG_ARGPARSE_HPP_CQUZWBHFS

#include "veg/assert.hpp"
#include "veg/internal/type_traits.hpp"
#include "veg/internal/meta_int.hpp"
#include "veg/fn_ref.hpp"

#include "veg/internal/std.hpp"
#include "veg/slice.hpp"
#include "veg/option.hpp"

namespace veg {

struct ternary {

  constexpr ternary() noexcept = default;
  constexpr ternary /* NOLINT(hicpp-explicit-conversions) */ (
      ternary_e e) noexcept
      : val(e) {}

  constexpr friend auto operator==(ternary a, ternary b) noexcept -> bool {
    return a.val == b.val;
  }

  template <typename CharT, typename Traits>
  friend auto operator<<(std::basic_ostream<CharT, Traits>& out, ternary t)
      -> std::basic_ostream<CharT, Traits>& {
    switch (t.val) {
    case maybe:
      out.write("none", 4);
      break;
    case yes:
      out.write("yes", 3);
      break;
    case no:
      out.write("no", 2);
      break;
    default:
      out.write("unknown", 7);
    }

    return out;
  }

private:
  ternary_e val = maybe;
};

struct argparse;
struct argparse_option;
using argparse_callback = fn_ref<int(argparse*, argparse_option const*)>;

namespace _argparse {

template <typename T>
struct is_supported : meta::bool_constant<                                //
                          meta::is_same<std::nullptr_t, T>::value ||      //
                          meta::is_same<char const**, T>::value ||        //
                          meta::is_same<ternary*, T>::value ||            //
                          meta::is_same<bool*, T>::value ||               //
                                                                          //
                          meta::is_same<char*, T>::value ||               //
                                                                          //
                          meta::is_same<char signed*, T>::value ||        //
                          meta::is_same<short signed*, T>::value ||       //
                          meta::is_same<int signed*, T>::value ||         //
                          meta::is_same<long signed*, T>::value ||        //
                          meta::is_same<long long signed*, T>::value ||   //
                                                                          //
                          meta::is_same<char unsigned*, T>::value ||      //
                          meta::is_same<short unsigned*, T>::value ||     //
                          meta::is_same<int unsigned*, T>::value ||       //
                          meta::is_same<long unsigned*, T>::value ||      //
                          meta::is_same<long long unsigned*, T>::value || //
                                                                          //
                          meta::is_same<float*, T>::value ||              //
                          meta::is_same<double*, T>::value ||             //
                          meta::is_same<long double*, T>::value           //
                          > {};

enum struct argparse_option_type {
  /* special */
  ARGPARSE_OPT_GROUP,
  /* options with no arguments */
  ARGPARSE_OPT_BOOLEAN,
  ARGPARSE_OPT_TERNARY,
  /* options with arguments (optional or required) */
  ARGPARSE_OPT_CHAR,

  ARGPARSE_OPT_UCHAR,
  ARGPARSE_OPT_USHORT,
  ARGPARSE_OPT_UINT,
  ARGPARSE_OPT_ULONG,
  ARGPARSE_OPT_ULONG_LONG,
  ARGPARSE_OPT_SCHAR,
  ARGPARSE_OPT_SSHORT,
  ARGPARSE_OPT_SINT,
  ARGPARSE_OPT_SLONG,
  ARGPARSE_OPT_SLONG_LONG,

  ARGPARSE_OPT_FLOAT,
  ARGPARSE_OPT_DOUBLE,
  ARGPARSE_OPT_LONG_DOUBLE,
  ARGPARSE_OPT_STRING,
};
template <typename T>
struct to_option_type {};

template <>
struct to_option_type<std::nullptr_t> {
  static constexpr _argparse::argparse_option_type value =
      _argparse::argparse_option_type::ARGPARSE_OPT_BOOLEAN; // unused
};

template <>
struct to_option_type<char> {
  static constexpr _argparse::argparse_option_type value =
      _argparse::argparse_option_type::ARGPARSE_OPT_CHAR;
};

template <>
struct to_option_type<char unsigned> {
  static constexpr _argparse::argparse_option_type value =
      _argparse::argparse_option_type::ARGPARSE_OPT_UCHAR;
};
template <>
struct to_option_type<short unsigned> {
  static constexpr _argparse::argparse_option_type value =
      _argparse::argparse_option_type::ARGPARSE_OPT_USHORT;
};
template <>
struct to_option_type<int unsigned> {
  static constexpr _argparse::argparse_option_type value =
      _argparse::argparse_option_type::ARGPARSE_OPT_UINT;
};
template <>
struct to_option_type<long unsigned> {
  static constexpr _argparse::argparse_option_type value =
      _argparse::argparse_option_type::ARGPARSE_OPT_ULONG;
};
template <>
struct to_option_type<long long unsigned> {
  static constexpr _argparse::argparse_option_type value =
      _argparse::argparse_option_type::ARGPARSE_OPT_ULONG_LONG;
};

template <>
struct to_option_type<signed char> {
  static constexpr _argparse::argparse_option_type value =
      _argparse::argparse_option_type::ARGPARSE_OPT_SCHAR;
};
template <>
struct to_option_type<short> {
  static constexpr _argparse::argparse_option_type value =
      _argparse::argparse_option_type::ARGPARSE_OPT_SSHORT;
};
template <>
struct to_option_type<int> {
  static constexpr _argparse::argparse_option_type value =
      _argparse::argparse_option_type::ARGPARSE_OPT_SINT;
};
template <>
struct to_option_type<long> {
  static constexpr _argparse::argparse_option_type value =
      _argparse::argparse_option_type::ARGPARSE_OPT_SLONG;
};
template <>
struct to_option_type<long long> {
  static constexpr _argparse::argparse_option_type value =
      _argparse::argparse_option_type::ARGPARSE_OPT_SLONG_LONG;
};

template <>
struct to_option_type<bool> {
  static constexpr _argparse::argparse_option_type value =
      _argparse::argparse_option_type::ARGPARSE_OPT_BOOLEAN;
};
template <>
struct to_option_type<ternary> {
  static constexpr _argparse::argparse_option_type value =
      _argparse::argparse_option_type::ARGPARSE_OPT_TERNARY;
};
template <>
struct to_option_type<float> {
  static constexpr _argparse::argparse_option_type value =
      _argparse::argparse_option_type::ARGPARSE_OPT_FLOAT;
};
template <>
struct to_option_type<double> {
  static constexpr _argparse::argparse_option_type value =
      _argparse::argparse_option_type::ARGPARSE_OPT_DOUBLE;
};
template <>
struct to_option_type<long double> {
  static constexpr _argparse::argparse_option_type value =
      _argparse::argparse_option_type::ARGPARSE_OPT_LONG_DOUBLE;
};
template <>
struct to_option_type<char const*> {
  static constexpr _argparse::argparse_option_type value =
      _argparse::argparse_option_type::ARGPARSE_OPT_STRING;
};

struct layout {
  _argparse::argparse_option_type type;
  const char short_name;
  char const* long_name;
  void* value;
  char const* help;
  option<argparse_callback> callback;
  unsigned flags;
};

auto argparse_parse(argparse* self, int argc, char** argv) -> int;
} // namespace _argparse
enum argparse_option_flags : unsigned {
  OPT_NONEG = 1, /* disable negation */
};

enum argparse_flag : unsigned {
  ARGPARSE_STOP_AT_NON_OPTION = 1,
};

/**
 *  argparse option
 *
 *  `type`:
 *    holds the type of the option.
 *
 *  `short_name`:
 *    single character to be used as a short option name, '\0' if none.
 *
 *  `long_name`:
 *    the long option name, without the leading dash, nullptr if none.
 *
 *  `value`:
 *    stores pointer to the value to be filled.
 *
 *  `help`:
 *    the short help message associated to what the option does.
 *    Must never be nullptr.
 *
 *  `callback`:
 *    function_ref is called when the corresponding argument is parsed.
 *
 *  `flags`:
 *    option flags.
 */

struct argparse_option : _argparse::layout {

  argparse_option /* NOLINT(hicpp-explicit-conversions) */ (
      char const* group_name) noexcept
      : argparse_option{
            {_argparse::argparse_option_type::ARGPARSE_OPT_GROUP,
             0,
             nullptr,
             nullptr,
             group_name,
             {},
             0}} {}

  VEG_TEMPLATE(
      (typename T),
      requires _argparse::is_supported<T>::value,
      argparse_option,
      (value_ptr, T),
      (arg_long_name, char const*),
      (help_str = "", char const*),
      (callback_fn = {}, option<argparse_callback>))
  noexcept
      : argparse_option{{
            _argparse::to_option_type<meta::remove_pointer_t<T>>::value,
            '\0',
            arg_long_name,
            value_ptr,
            help_str,
            callback_fn,
            0,
        }} {}

  VEG_TEMPLATE(
      (typename T),
      requires _argparse::is_supported<T>::value,
      argparse_option,
      (value_ptr, T),
      (arg_short_name, char),
      (arg_long_name = nullptr, char const*),
      (help_str = "", char const*),
      (callback_fn = {}, option<argparse_callback>))
  noexcept
      : argparse_option{{
            _argparse::to_option_type<meta::remove_pointer_t<T>>::value,
            arg_short_name,
            arg_long_name,
            value_ptr,
            help_str,
            callback_fn,
            0,
        }} {}

  explicit argparse_option(layout l) noexcept : layout{l} {}
};

/**
 * argpparse
 */

struct argparse {
  int argc;
  char** argv;
  argparse_option const* const options;
  i64 const argparse_options_len;
  char const* const* const usages;
  i64 const usages_len;
  unsigned const flags;
  char const* const description;
  char const* const epilogue;
  char** out;
  int cpidx;
  char const* optvalue; // current option value
};

inline void parse_args(
    int* argc,
    char** argv,
    slice<argparse_option const> options,
    slice<char const* const> usages,
    char const* description = "",
    char const* epilogue = "",
    unsigned flags = 0) noexcept {
  argparse ap = {
      *argc,
      argv,
      options.data(),
      options.size(),
      usages.data(),
      usages.size(),
      flags,
      description,
      epilogue,
      nullptr,
      0,
      nullptr};
  *argc = _argparse::argparse_parse(&ap, *argc, argv);
}

// built-in callbacks
auto argparse_help_cb(argparse* self, argparse_option const* option) -> int;

static const argparse_option help = argparse_option{
    {_argparse::argparse_option_type::ARGPARSE_OPT_BOOLEAN,
     'h',
     "help",
     nullptr,
     "show this help message and exit",
     {some, argparse_help_cb},
     OPT_NONEG}};

} // namespace veg
#endif /* end of include guard VEG_ARGPARSE_HPP_CQUZWBHFS */
