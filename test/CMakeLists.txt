include_directories(include)

add_library(backward_cpp_main OBJECT src/backward.cpp)
target_link_libraries(backward_cpp_main CONAN_PKG::backward-cpp)
set(testlibs veg backward_cpp_main CONAN_PKG::gtest)

add_executable(test.null src/null.cpp)
target_link_libraries(test.null ${testlibs})

add_executable(test.int src/test_int_c.cpp)
target_link_libraries(test.int ${testlibs})

add_executable(test.fn src/test_function_ref.cpp)
target_link_libraries(test.fn ${testlibs})

add_executable(test.arg src/test_argparse.cpp)
target_link_libraries(test.arg ${testlibs})

add_executable(test.dynstack src/test_dynamic_stack.cpp)
target_link_libraries(test.dynstack ${testlibs})

add_executable(test.tuple src/test_tuple.cpp)
target_link_libraries(test.tuple ${testlibs})

add_executable(test.option src/test_option.cpp)
target_link_libraries(test.option ${testlibs})

add_executable(test.option.stx src/test_option.stx.cpp)
target_link_libraries(test.option.stx ${testlibs})

gtest_discover_tests(test.int)
gtest_discover_tests(test.fn)
gtest_discover_tests(test.arg)
gtest_discover_tests(test.dynstack)
gtest_discover_tests(test.tuple)
gtest_discover_tests(test.option)
gtest_discover_tests(test.option.stx)

add_custom_target(
  check
  COMMAND ${CMAKE_CTEST_COMMAND} --verbose
  DEPENDS test.null
          test.int
          test.fn
          test.arg
          test.dynstack
          test.tuple
          test.option
          test.option.stx
)
