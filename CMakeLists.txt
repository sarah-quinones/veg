cmake_minimum_required(VERSION 3.16)

set(CMAKE_DEBUG_POSTFIX "-d") # append -d to debug targets so they can be installed alongside release ones

set(veg_version 0.1)
project(veg LANGUAGES CXX VERSION ${veg_version})

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

add_library(
  veg
  ${CMAKE_CURRENT_SOURCE_DIR}/src/veg.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/time.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/parse.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ub.cpp
)
target_include_directories(veg SYSTEM INTERFACE
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_include_directories(veg PRIVATE include)

option(VEG_TESTS "enable tests" OFF)

if(VEG_TESTS)
  add_library(project_warnings INTERFACE)
  add_library(project_options INTERFACE)
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/compiler_warnings.cmake)
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/static_analyzers.cmake)
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/sanitizers.cmake)
  target_link_libraries(veg INTERFACE project_options project_warnings)
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/extra_local_settings.cmake)
  enable_sanitizers(project_options)

  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/conan.cmake)
  # Conan dependencies
  set(CONAN_REQUIRES fmt/8.0.1 backward-cpp/1.5 benchmark/1.6.0)
  set(CONAN_OPTIONS)
  run_conan()
  target_link_libraries(veg PUBLIC CONAN_PKG::fmt)

  set_project_warnings(project_warnings)

  enable_testing()
  add_subdirectory(test)
  add_subdirectory(benchmark)
endif()

option(VEG_INSTALL "enable install" ON)

if(VEG_INSTALL)
  include(GNUInstallDirs)
  include(CMakePackageConfigHelpers)
  install(
    TARGETS veg EXPORT veg-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
  install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include/)
  install(EXPORT veg-targets
    FILE veg-targets.cmake
    NAMESPACE veg::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/veg
  )
  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/veg-config-version.cmake"
    VERSION "${veg_version}"
    COMPATIBILITY AnyNewerVersion
  )
  configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/veg-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/veg
  )
  install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/veg-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/veg-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/veg
  )
endif()
