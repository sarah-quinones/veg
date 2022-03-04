if(PROJECT_IS_TOP_LEVEL)
  set(CMAKE_INSTALL_INCLUDEDIR include/veg CACHE PATH "")
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package veg)

install(
    DIRECTORY
    include/
    "${PROJECT_BINARY_DIR}/export/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT veg_Development
)

install(
    TARGETS veg_veg
    EXPORT vegTargets
    RUNTIME #
    COMPONENT veg_Runtime
    LIBRARY #
    COMPONENT veg_Runtime
    NAMELINK_COMPONENT veg_Development
    ARCHIVE #
    COMPONENT veg_Development
    INCLUDES #
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

configure_file(
    cmake/install-config.cmake.in "${package}Config.cmake"
    @ONLY
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
    veg_INSTALL_CMAKEDIR "${CMAKE_INSTALL_DATADIR}/${package}"
    CACHE PATH "CMake package config location relative to the install prefix"
)
mark_as_advanced(veg_INSTALL_CMAKEDIR)

install(
    FILES
    "${PROJECT_BINARY_DIR}/${package}Config.cmake"
    "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${veg_INSTALL_CMAKEDIR}"
    COMPONENT veg_Development
)

install(
    EXPORT vegTargets
    NAMESPACE veg::
    DESTINATION "${veg_INSTALL_CMAKEDIR}"
    COMPONENT veg_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
