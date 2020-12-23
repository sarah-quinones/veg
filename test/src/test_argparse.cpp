#include "veg/argparse.hpp"
#include <gtest/gtest.h>

#include <cstdio>
#include <cstring>

TEST(argparse, all) {
  int argc = 10;

  char arg0[] = "/dir/exec";
  char arg1[] = "--path";
  char arg2[] = "/a/b/c";
  char arg3[] = "--float";
  char arg4[] = "49.5e3";
  char arg5[] = "-r";
  char arg6[] = "--";
  char arg7[] = "1";
  char arg8[] = "text";
  char arg9[] = "2";

  char* argv[] = {
      arg0,
      arg1,
      arg2,
      arg3,
      arg4,
      arg5,
      arg6,
      arg7,
      arg8,
      arg9,
  };

  veg::ternary tern;
  bool force = true;
  long num = 0;
  long double flt = 0;
  char a = 'a';
  char const* path = "default";

  char const* usage[] = {
      "test_argparse [options] [[--] args]",
      "test_argparse [options]",
  };
  bool found = false;

  veg::parse_args(
      &argc,
      argv,
      {veg::init_list,
       {
           veg::help,
           "Basic options",
           {&a, 'c', "char"},

           {nullptr,
            'r',
            "tern",
            "",
            {veg::some,
             [&found](
                 veg::argparse* /*self*/,
                 veg::argparse_option const* /*opt*/) -> int {
               found = true;
               return 0;
             }}},

           {&force, 'f', "force", "force to do"},
           "More options",
           {&path, 'p', "path", "path to read"},
           {&flt, "float", "num"},
           {&num, "num", "selected num"},
       }},
      usage,
      "description",
      "more description");

  EXPECT_EQ(tern, veg::maybe);
  EXPECT_STREQ(path, "/a/b/c");
  EXPECT_EQ(flt, 49.5e3L);
  EXPECT_TRUE(found);

  EXPECT_EQ(argc, 3);
  EXPECT_STREQ(argv[0], "1");
  EXPECT_STREQ(argv[1], "text");
  EXPECT_STREQ(argv[2], "2");
}
