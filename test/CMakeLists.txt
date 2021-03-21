include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/doctest.cmake)

add_library(backward_cpp_main OBJECT src/backward.cpp)
target_include_directories(
  backward_cpp_main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(backward_cpp_main CONAN_PKG::backward-cpp)

add_library(doctest_cpp_main src/doctest_main.cpp)
target_include_directories(
  doctest_cpp_main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(veg.all INTERFACE)
target_include_directories(
  veg.all INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(veg.all INTERFACE veg doctest_cpp_main backward_cpp_main)

macro(
  add_test_std_version
  std_version
  name
  filename
)
  add_executable(test.${std_version}.${name} ${filename})
  target_link_libraries(test.${std_version}.${name} veg.${std_version})
  set_target_properties(
    test.${std_version}.${name} PROPERTIES CXX_EXTENSIONS OFF
  )
  doctest_discover_tests(
    test.${std_version}.${name} TEST_PREFIX test.${std_version}.
  )

  set(test_list.${std_version} ${test_list.${std_version}}
                               test.${std_version}.${name}
  )
  set(test_list ${test_list} test.${std_version}.${name})
endmacro()

macro(add_all_tests_std_version std_version)
  add_library(veg.${std_version} INTERFACE)
  target_include_directories(
    veg.${std_version} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
  )
  target_link_libraries(veg.${std_version} INTERFACE veg.all)
  target_compile_features(veg.${std_version} INTERFACE cxx_std_${std_version})
  target_compile_definitions(
    veg.${std_version} INTERFACE -DVEG_INTERNAL_ASSERTIONS
  )

  add_test_std_version(${std_version} int src/test_int_c.cpp)
  add_test_std_version(${std_version} fn src/test_fn_view.cpp)
  add_test_std_version(${std_version} dynstack src/test_dynamic_stack.cpp)
  add_test_std_version(${std_version} tuple src/test_tuple.cpp)
  add_test_std_version(${std_version} option src/test_option.cpp)
  add_test_std_version(${std_version} option.stx src/test_option.stx.cpp)

  add_custom_target(
    check.${std_version}
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose --tests-regex test.${std_version}
    DEPENDS ${test_list.${std_version}}
  )

endmacro()

add_all_tests_std_version(11)
add_all_tests_std_version(14)
add_all_tests_std_version(17)
add_all_tests_std_version(20)

add_custom_target(
  check
  COMMAND ${CMAKE_CTEST_COMMAND} --verbose
  DEPENDS ${test_list}
)
