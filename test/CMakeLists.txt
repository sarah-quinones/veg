include(../cmake/doctest.cmake)

include_directories(include)

add_library(doctest_main STATIC src/doctest_main.cpp)
add_library(backward_cpp_main OBJECT src/backward.cpp)
target_link_libraries(backward_cpp_main CONAN_PKG::backward-cpp)
set(testlibs veg backward_cpp_main doctest_main)

add_executable(test.null src/null.cpp)
target_link_libraries(test.null veg ${testlibs})

add_executable(test.int src/test_int_c.cpp)
target_link_libraries(test.int veg ${testlibs})

add_executable(test.fn src/test_function_ref.cpp)
target_link_libraries(test.fn veg ${testlibs})

add_executable(test.arg src/test_argparse.cpp)
target_link_libraries(test.arg veg ${testlibs})

add_executable(test.dynstack src/test_dynamic_stack.cpp)
target_link_libraries(test.dynstack veg ${testlibs})

add_executable(test.tuple src/test_tuple.cpp)
target_link_libraries(test.tuple veg ${testlibs})

doctest_discover_tests(test.int)
doctest_discover_tests(test.fn)
doctest_discover_tests(test.arg)
doctest_discover_tests(test.dynstack)
doctest_discover_tests(test.tuple)

add_custom_target(
  check
  COMMAND ${CMAKE_CTEST_COMMAND} --verbose
  DEPENDS test.null
          test.int
          test.fn
          test.arg
          test.dynstack
          test.tuple
)
