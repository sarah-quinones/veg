include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/doctest.cmake)

add_library(doctest_cpp_main src/doctest_main.cpp)
target_include_directories(
  doctest_cpp_main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(doctest_cpp_main CONAN_PKG::backward-cpp)

add_library(veg.all INTERFACE)
target_include_directories(
  veg.all INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(veg.all INTERFACE veg doctest_cpp_main)

macro(set_cxx_version target std_version)
  set_target_properties(${target} PROPERTIES CXX_STANDARD ${std_version})
  set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)
endmacro()

macro(
  add_test_std_version
  std_version
  name
  filename
)
  add_executable(test.${std_version}.${name} ${filename})
  target_link_libraries(test.${std_version}.${name} veg.${std_version})
  set_cxx_version(test.${std_version}.${name} ${std_version})
  doctest_discover_tests(
    test.${std_version}.${name} TEST_PREFIX test.${std_version}.
  )

  set(test_list.${std_version} ${test_list.${std_version}}
                               test.${std_version}.${name}
  )
  set(test_list ${test_list} test.${std_version}.${name})
endmacro()

macro(add_all_tests_std_version std_version)
  add_library(veg.${std_version} OBJECT src/null.cpp)
  target_include_directories(
    veg.${std_version} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  )
  target_compile_definitions(veg.${std_version} PUBLIC __VEG_DISABLE_NOEXCEPT)
  target_link_libraries(veg.${std_version} veg.all)
  set_cxx_version(veg.${std_version} ${std_version})
  target_compile_definitions(
    veg.${std_version} PUBLIC -D__VEG_INTERNAL_ASSERTIONS
  )

  add_test_std_version(${std_version} idx src/test_index.cpp)
  add_test_std_version(${std_version} fn src/test_fn_view.cpp)
  add_test_std_version(${std_version} dynstack src/test_dynamic_stack.cpp)
  add_test_std_version(${std_version} tuple src/test_tuple.cpp)
  add_test_std_version(${std_version} option src/test_option.cpp)
  add_test_std_version(${std_version} uwunion src/test_uwunion.cpp)
  add_test_std_version(${std_version} option.stx src/test_option.stx.cpp)
  add_test_std_version(${std_version} vec src/vec.cpp)

  add_custom_target(
    check.${std_version}
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose --tests-regex test.${std_version}.
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
