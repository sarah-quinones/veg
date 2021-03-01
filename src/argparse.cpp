/**
 * Copyright (c) 2020 sarah k.
 * Copyright (C) 2012-2015 Yecheng Fu <cofyc.jackson at gmail dot com>
 * All rights reserved.
 *
 * Use of this source code is governed by a MIT-style license that can be found
 * in the LICENSE file.
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <limits>

#include "veg/argparse.hpp"
#include "veg/assert.hpp"
#include "veg/internal/narrow.hpp"

namespace veg {

using namespace _argparse;

void argparse_usage(argparse const* self);

#define OPT_UNSET 1U
#define OPT_LONG (1U << 1U)

static auto prefix_skip(char const* str, char const* prefix) -> char const* {
  size_t len = std::strlen(prefix);
  return std::strncmp(str, prefix, len) != 0 ? nullptr : str + len;
}

static auto prefix_cmp(char const* str, char const* prefix) -> int {
  for (;; str++, prefix++) {
    if (*prefix == 0) {
      return 0;
    } else if (*str != *prefix) {
      return static_cast<unsigned char>(*prefix) -
             static_cast<unsigned char>(*str);
    }
  }
}

static void argparse_error(
    argparse* self,
    argparse_option const* opt,
    char const* reason,
    unsigned flags) {
  (void)self;
  if ((flags & OPT_LONG) != 0) {
    std::fprintf(stderr, "error: option `--%s` %s\n", opt->long_name, reason);
  } else {
    std::fprintf(stderr, "error: option `-%c` %s\n", opt->short_name, reason);
  }
  exit(1);
}

template <typename T>
auto as_ref(void* ptr) -> T& {
  return *static_cast<T*>(ptr);
}

template <
    typename T,
    meta::enable_if_t<
        std::is_integral<T>::value and std::is_signed<T>::value>* = nullptr>
auto parse_fn(char const* in, char** str_end) -> T {
  long long val = std::strtoll(in, str_end, 0);
  if (val == static_cast<long long>(static_cast<T>(val))) {
    return static_cast<T>(val);
  }
  errno = ERANGE;
  return val > 0 ? std::numeric_limits<T>::max()
                 : std::numeric_limits<T>::lowest();
}
template <
    typename T,
    meta::enable_if_t<
        std::is_integral<T>::value and std::is_unsigned<T>::value>* = nullptr>
auto parse_fn(char const* in, char** str_end) -> T {
  long long unsigned val = std::strtoull(in, str_end, 0);
  if (val == static_cast<long long unsigned>(static_cast<T>(val))) {
    return static_cast<T>(val);
  }
  errno = ERANGE;
  return std::numeric_limits<T>::max();
}

template <
    typename T,
    meta::enable_if_t<std::is_floating_point<T>::value>* = nullptr>
auto parse_fn(char const* in, char** str_end) -> T;

template <>
auto parse_fn(char const* in, char** str_end) -> float {
  return std::strtof(in, str_end);
}
template <>
auto parse_fn(char const* in, char** str_end) -> double {
  return std::strtod(in, str_end);
}
template <>
auto parse_fn(char const* in, char** str_end) -> long double {
  return std::strtold(in, str_end);
}

template <typename T>
void parse_num(
    argparse*& self,
    argparse_option const* opt,
    unsigned const flags,
    char*& s) {
  errno = 0;
  if (self->optvalue != nullptr) {
    as_ref<T>(opt->value) = parse_fn<T>(self->optvalue, &s);
    self->optvalue = nullptr;
  } else if (self->argc > 1) {
    self->argc--;
    as_ref<T>(opt->value) = parse_fn<T>(*++self->argv, &s);
  } else {
    argparse_error(self, opt, "requires a value", flags);
  }
  if (errno) {
    argparse_error(self, opt, std::strerror(errno), flags);
  }
  if (s[0] != '\0') {
    argparse_error(self, opt, "expects an integer value", flags);
  }
}

static auto argparse_getvalue(
    argparse* self, argparse_option const* opt, unsigned const flags) -> int {
  using Char = char;
  Char* s = nullptr;
  if (opt->value == nullptr) {
    VEG_ASSERT(opt->callback);
    return (opt->callback).as_ref().unwrap()(self, opt);
  }
  switch (opt->type) {
  case to_option_type<ternary>::value:
    as_ref<ternary>(opt->value) = ((flags & OPT_UNSET) != 0) ? no : yes;
    break;
  case to_option_type<bool>::value:
    as_ref<bool>(opt->value) = !((flags & OPT_UNSET) != 0);
    break;
  case to_option_type<char const*>::value:
    if (self->optvalue != nullptr) {
      as_ref<char const*>(opt->value) = self->optvalue;
      self->optvalue = nullptr;
    } else if (self->argc > 1) {
      self->argc--;
      as_ref<char const*>(opt->value) = *++self->argv;
    } else {
      argparse_error(self, opt, "requires a value", flags);
    }
    break;

  case to_option_type<char>::value:
    if (self->optvalue != nullptr) {
      as_ref<char>(opt->value) = self->optvalue[0];
      if (self->optvalue[0] != '\0' && self->optvalue[1] != '\0') {
        argparse_error(self, opt, "requires a single character", flags);
      }
      self->optvalue = nullptr;
    } else if (self->argc > 1) {
      self->argc--;
      ++(self->argv);
      as_ref<char>(opt->value) = (*self->argv)[0];
      if ((*self->argv)[0] != '\0' && (*self->argv)[1] != '\0') {
        argparse_error(self, opt, "requires a single character", flags);
      }
    } else {
      argparse_error(self, opt, "requires a value", flags);
    }
    break;

#undef PARSE_NUM
#define PARSE_NUM(T)                                                           \
  case to_option_type<T>::value:                                               \
    parse_num<T>(self, opt, flags, s);                                         \
    break

    PARSE_NUM(char unsigned);
    PARSE_NUM(short unsigned);
    PARSE_NUM(int unsigned);
    PARSE_NUM(long unsigned);
    PARSE_NUM(long long unsigned);
    PARSE_NUM(char signed);
    PARSE_NUM(short);
    PARSE_NUM(int);
    PARSE_NUM(long);
    PARSE_NUM(long long);
    PARSE_NUM(float);
    PARSE_NUM(double);
    PARSE_NUM(long double);
#undef PARSE_NUM

  default:
    VEG_ASSERT_ELSE("unexpected", false);
  }

  if (opt->callback) {
    return opt->callback.as_ref().unwrap()(self, opt);
  }

  return 0;
}

static void argparse_options_check(
    argparse_option const* options, i64 argparse_options_len) {
  for (i64 i = 0; i < argparse_options_len; ++i) {
    auto const* option = options + i;
    switch (option->type) {
    case to_option_type<ternary>::value:
    case to_option_type<bool>::value:

    case to_option_type<char>::value:

    case to_option_type<char unsigned>::value:
    case to_option_type<short unsigned>::value:
    case to_option_type<int unsigned>::value:
    case to_option_type<long unsigned>::value:
    case to_option_type<long long unsigned>::value:
    case to_option_type<char signed>::value:
    case to_option_type<short signed>::value:
    case to_option_type<int signed>::value:
    case to_option_type<long signed>::value:
    case to_option_type<long long signed>::value:

    case to_option_type<float>::value:
    case to_option_type<double>::value:
    case to_option_type<long double>::value:
    case to_option_type<char const*>::value:
    case argparse_option_type::ARGPARSE_OPT_GROUP:
      continue;
    default:
      std::fprintf(stderr, "wrong option type: %d\n", int(option->type));
      break;
    }
  }
}

static auto argparse_short_opt(argparse* self, argparse_option const* options)
    -> int {
  for (; options < self->options + self->argparse_options_len; options++) {
    if (options->short_name == *self->optvalue) {
      self->optvalue = self->optvalue[1] != 0 ? self->optvalue + 1 : nullptr;
      return argparse_getvalue(self, options, 0);
    }
  }
  return -2;
}

static auto argparse_long_opt(argparse* self, argparse_option const* options)
    -> int {
  for (; options < self->options + self->argparse_options_len; options++) {
    char const* rest = nullptr;
    unsigned opt_flags = 0;
    if (options->long_name == nullptr) {
      continue;
    }

    rest = prefix_skip(self->argv[0] + 2, options->long_name);
    if (rest == nullptr) {
      // negation disabled?
      if ((options->flags & OPT_NONEG) != 0) {
        continue;
      }
      // only OPT_BOOLEAN/OPT_BIT supports negation
      if (options->type != to_option_type<ternary>::value &&
          options->type != to_option_type<bool>::value) {
        continue;
      }

      if (prefix_cmp(self->argv[0] + 2, "no-") != 0) {
        continue;
      }
      rest = prefix_skip(self->argv[0] + 2 + 3, options->long_name);
      if (rest == nullptr) {
        continue;
      }
      opt_flags |= OPT_UNSET;
    }
    if (*rest != 0) {
      if (*rest != '=') {
        continue;
      }
      self->optvalue = rest + 1;
    }
    return argparse_getvalue(self, options, opt_flags | OPT_LONG);
  }
  return -2;
}

auto _argparse::argparse_parse(argparse* self, int argc, char** argv) -> int {
  self->argc = argc - 1;
  self->argv = argv + 1;
  self->out = argv;

  auto finish = [&] {
    std::memmove(
        self->out + self->cpidx,
        self->argv,
        fn::narrow<std::size_t>{}(self->argc) * sizeof(*self->out));
    self->out[self->cpidx + self->argc] = nullptr;

    return self->cpidx + self->argc;
  };
  auto unknown = [&] {
    std::fprintf(stderr, "error: unknown option `%s`\n", self->argv[0]);
    argparse_usage(self);
    std::exit(1);
  };

  argparse_options_check(self->options, self->argparse_options_len);

  for (; self->argc != 0; self->argc--, self->argv++) {
    char const* arg = self->argv[0];
    if (arg[0] != '-' || (arg[1] == '\0')) {
      if ((self->flags & ARGPARSE_STOP_AT_NON_OPTION) != 0) {
        return finish();
      }
      // if it's not option or is a single char '-', copy verbatim
      self->out[self->cpidx++] = self->argv[0];
      continue;
    }
    // short option
    if (arg[1] != '-') {
      self->optvalue = arg + 1;
      switch (argparse_short_opt(self, self->options)) {
      case -1:
        break;
      case -2:
        unknown();
      }
      while (self->optvalue != nullptr) {
        switch (argparse_short_opt(self, self->options)) {
        case -1:
          break;
        case -2:
          unknown();
        }
      }
      continue;
    }
    // if '--' presents
    if (arg[2] == '\0') {
      self->argc--;
      self->argv++;
      break;
    }
    // long option
    switch (argparse_long_opt(self, self->options)) {
    case -1:
      break;
    case -2:
      unknown();
    }
  }

  return finish();
}

void argparse_usage(argparse const* self) {
  char const* const* const usages_first = self->usages;
  char const* const* usages = usages_first;
  if (self->usages != nullptr) {
    std::fprintf(stdout, "Usage: %s\n", *self->usages);
    ++usages;
    while (self->usages < usages + self->usages_len) {
      std::fprintf(stdout, "   or: %s\n", *usages);
      ++usages;
    }
  } else {
    std::fprintf(stdout, "Usage:\n");
  }

  // print description
  if (self->description != nullptr) {
    std::fprintf(stdout, "%s\n", self->description);
  }

  std::fputc('\n', stdout);

  // figure out best width
  i64 usage_opts_width = 0;
  i64 len = 0;
  for (i64 i = 0; i < self->argparse_options_len; ++i) {
    auto const* options = self->options + i;
    len = 0;
    if ((options)->short_name != 0) {
      len += 2;
    }
    if (((options)->short_name != 0) && ((options)->long_name != nullptr)) {
      len += 2; // separator ", "
    }
    if ((options)->long_name != nullptr) {
      len += fn::narrow<i64>{}(std::strlen(options->long_name)) + 2;
    }

    if (options->type >= to_option_type<char>::value) {
      len += fn::narrow<i64>{}(std::strlen("=<char>"));
    } else if (options->type == to_option_type<char const*>::value) {
      len += fn::narrow<i64>{}(std::strlen("=<str>"));
    } else if (options->type >= to_option_type<float>::value) {
      len += fn::narrow<i64>{}(std::strlen("=<flt>"));
    } else if (options->type >= to_option_type<unsigned char>::value) {
      len += fn::narrow<i64>{}(std::strlen("=<int>"));
    }
    len = (len + 3) - ((len + 3) % 4);
    if (usage_opts_width < len) {
      usage_opts_width = len;
    }
  }
  usage_opts_width += 4; // 4 spaces prefix

  for (i64 i = 0; i < self->argparse_options_len; ++i) {
    auto const* options = self->options + i;
    i64 pos = 0;
    i64 pad = 0;
    if (options->type == argparse_option_type::ARGPARSE_OPT_GROUP) {
      std::fputc('\n', stdout);
      std::fprintf(stdout, "%s", options->help);
      std::fputc('\n', stdout);
      continue;
    }
    pos = std::fprintf(stdout, "    ");
    if (options->short_name != 0) {
      pos += std::fprintf(stdout, "-%c", options->short_name);
    }
    if ((options->long_name != nullptr) && (options->short_name != 0)) {
      pos += std::fprintf(stdout, ", ");
    }
    if (options->long_name != nullptr) {
      pos += std::fprintf(stdout, "--%s", options->long_name);
    }

    if (options->type == to_option_type<char>::value) {
      pos += std::fprintf(stdout, "=<char>");
    } else if (options->type == to_option_type<char const*>::value) {
      pos += std::fprintf(stdout, "=<str>");
    } else if (options->type >= to_option_type<float>::value) {
      pos += std::fprintf(stdout, "=<flt>");
    } else if (options->type >= to_option_type<char unsigned>::value) {
      pos += std::fprintf(stdout, "=<int>");
    }
    if (pos <= usage_opts_width) {
      pad = usage_opts_width - pos;
    } else {
      std::fputc('\n', stdout);
      pad = usage_opts_width;
    }
    std::fprintf(stdout, "%*s%s\n", int(pad + 2), "", options->help);
  }

  // print epilogue
  if (self->epilogue != nullptr) {
    std::fprintf(stdout, "%s\n", self->epilogue);
  }
}

auto argparse_help_cb(argparse* self, argparse_option const* option) -> int {
  (void)option;
  argparse_usage(self);
  std::exit(0);
}

} // namespace veg
