cmake_minimum_required(VERSION 3.16)
project(veg CXX)

get_directory_property(has_parent PARENT_DIRECTORY)
if(has_parent)
  set(VEG_TOP_LEVEL FALSE)
else()
  set(VEG_TOP_LEVEL TRUE)
endif()

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

add_library(_veg_ub ${CMAKE_CURRENT_SOURCE_DIR}/src/ub.cpp)
set_property(TARGET _veg_ub PROPERTY INTERPROCEDURAL_OPTIMIZATION FALSE)
add_library(
  veg
  ${CMAKE_CURRENT_SOURCE_DIR}/src/veg.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/alloc.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/time.cpp
)
target_link_libraries(veg PRIVATE _veg_ub)
target_include_directories(veg PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

option(VEG_ENABLE_TESTING "enable tests" ${VEG_TOP_LEVEL})

if(VEG_ENABLE_TESTING)
  enable_testing()
  add_subdirectory(test)
endif()
