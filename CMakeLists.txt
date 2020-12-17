cmake_minimum_required(VERSION 3.16)
project(veg CXX)

get_directory_property(has_parent PARENT_DIRECTORY)
if(has_parent)
  set(top_level FALSE)
else()
  set(top_level TRUE)
endif()

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

if(ENABLE_DOCS_ONLY)
  add_subdirectory("docs")
  return()
endif()

if(top_level AND ENABLE_TESTING)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

include(cmake/compiler_warnings.cmake)
include(cmake/static_analyzers.cmake)
include(cmake/sanitizers.cmake)
include(cmake/conan.cmake)

add_library(veg src/veg.cpp src/argparse.cpp)
target_include_directories(veg PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(top_level AND ENABLE_TESTING)
  # Conan dependencies
  set(CONAN_REQUIRES # MIT License
      fmt/7.1.2 backward-cpp/1.5
  )
  set(CONAN_OPTIONS)
  run_conan()

  target_link_libraries(veg PUBLIC CONAN_PKG::fmt)

  add_library(project_warnings INTERFACE)
  add_library(project_options INTERFACE)

  target_link_libraries(veg INTERFACE project_options project_warnings)

  include(cmake/extra_local_settings.cmake)

  set_project_warnings(project_warnings)

  enable_sanitizers(project_options)
  option(ENABLE_PCH "Enable Precompiled Headers" OFF)
  if(ENABLE_PCH)
    target_precompile_headers(
      project_options
      INTERFACE
      <memory>
      <utility>
    )
  endif()

  enable_testing()
  add_subdirectory(test)
  add_subdirectory(benchmark)
endif()
