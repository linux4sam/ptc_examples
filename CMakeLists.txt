cmake_minimum_required(VERSION 3.23)

project(ptc_examples
    VERSION 1.3.1
    DESCRIPTION "PTC demos"
    LANGUAGES C
)
#set(PROJECT_VERSION_RC 1)
if(DEFINED PROJECT_VERSION_RC)
    string(APPEND PROJECT_VERSION "-rc${PROJECT_VERSION_RC}")
endif()

set(CMAKE_VERBOSE_MAKEFILE True)

find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBGPIOD REQUIRED libgpiod<2.0.0)
pkg_check_modules(LIBEVDEV REQUIRED libevdev)

add_subdirectory(src)
