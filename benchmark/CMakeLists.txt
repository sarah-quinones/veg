add_executable(.bench.vec src/vec.cpp)
target_link_libraries(.bench.vec veg CONAN_PKG::benchmark)

add_executable(.bench.uwunion src/uwunion.cpp)
target_link_libraries(.bench.uwunion veg CONAN_PKG::benchmark)
set_target_properties(.bench.uwunion PROPERTIES CXX_STANDARD 17)
set_target_properties(.bench.vec PROPERTIES CXX_STANDARD 17)
