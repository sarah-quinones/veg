cmake_minimum_required(VERSION 3.14)

project(vegTests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)

if(PROJECT_IS_TOP_LEVEL)
  find_package(veg REQUIRED)
  enable_testing()
endif()

find_package(Catch2 REQUIRED)
include(Catch)

macro(
  add_test_std_version
  std_version
  name
  filename
)
  add_executable(test.${std_version}.${name} ${filename})
  target_link_libraries(
    test.${std_version}.${name} PRIVATE veg::veg Catch2::Catch2WithMain
  )
  target_include_directories(test.${std_version}.${name} PRIVATE include)
  target_compile_features(
    test.${std_version}.${name} PRIVATE cxx_std_${std_version}
  )
  catch_discover_tests(
    test.${std_version}.${name} TEST_PREFIX test.${std_version}.
  )

  set(test_list.${std_version} ${test_list.${std_version}}
                               test.${std_version}.${name}
  )
  set(test_list ${test_list} test.${std_version}.${name})
endmacro()

macro(add_test_all_std_versions name filename)
  add_test_std_version(11 ${name} ${filename})
  add_test_std_version(14 ${name} ${filename})
  add_test_std_version(17 ${name} ${filename})
  add_test_std_version(20 ${name} ${filename})
endmacro()

add_test_all_std_versions(fn source/test_fn_view.cpp)
add_test_all_std_versions(idx source/test_index.cpp)
add_test_all_std_versions(option source/test_option.cpp)
add_test_all_std_versions(option.stx source/test_option.stx.cpp)
add_test_all_std_versions(tuple source/test_tuple.cpp)
add_test_all_std_versions(dynstack source/test_dynamic_stack.cpp)
add_test_all_std_versions(uwunion source/test_uwunion.cpp)

add_custom_target(
  check.11
  COMMAND ${CMAKE_CTEST_COMMAND} --verbose --tests-regex test.11.
  DEPENDS ${test_list.11}
)
add_custom_target(
  check.14
  COMMAND ${CMAKE_CTEST_COMMAND} --verbose --tests-regex test.14.
  DEPENDS ${test_list.14}
)
add_custom_target(
  check.17
  COMMAND ${CMAKE_CTEST_COMMAND} --verbose --tests-regex test.17.
  DEPENDS ${test_list.17}
)
add_custom_target(
  check.20
  COMMAND ${CMAKE_CTEST_COMMAND} --verbose --tests-regex test.20.
  DEPENDS ${test_list.20}
)
add_custom_target(
  check
  COMMAND ${CMAKE_CTEST_COMMAND} --verbose
  DEPENDS ${test_list}
)

add_folders(Test)
