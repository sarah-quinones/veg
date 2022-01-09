cmake_minimum_required(VERSION 3.16)
project(veg CXX)

get_directory_property(has_parent PARENT_DIRECTORY)

if(has_parent)
  set(VEG_TOP_LEVEL FALSE)
else()
  set(VEG_TOP_LEVEL TRUE)
endif()

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

if(VEG_DOCS)
  add_subdirectory(docs)
  return()
endif()

add_library(_veg_ub ${CMAKE_CURRENT_SOURCE_DIR}/src/ub.cpp)
set_property(TARGET _veg_ub PROPERTY INTERPROCEDURAL_OPTIMIZATION FALSE)
add_library(
  veg ${CMAKE_CURRENT_SOURCE_DIR}/src/veg.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/src/time.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/src/parse.cpp
)
target_include_directories(veg PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(_veg_ub PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(veg PRIVATE _veg_ub)

option(VEG_ENABLE_TESTING "enable tests" ${VEG_TOP_LEVEL})

if(VEG_ENABLE_TESTING)
  add_library(project_warnings INTERFACE)
  add_library(project_options INTERFACE)
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/compiler_warnings.cmake)
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/static_analyzers.cmake)
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/sanitizers.cmake)
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/conan.cmake)
  target_link_libraries(veg INTERFACE project_options project_warnings)
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/extra_local_settings.cmake)
  # Conan dependencies
  set(CONAN_REQUIRES fmt/8.0.1 backward-cpp/1.5 benchmark/1.6.0)
  set(CONAN_OPTIONS)
  run_conan()
  target_link_libraries(veg PUBLIC CONAN_PKG::fmt)

  enable_sanitizers(project_options)
  set_project_warnings(project_warnings)

  enable_testing()
  add_subdirectory(test)
  add_subdirectory(benchmark)
endif()
