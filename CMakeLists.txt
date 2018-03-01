cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(regexperience_core VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    # debug symbols stripping
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
    # optimizations
    add_compile_options(-O2)
endif()

include(GNUInstallDirs)

find_package(PkgConfig REQUIRED)

# GLib & GObject
pkg_check_modules(GLIB_GOBJECT glib-2.0>=2.48 gobject-2.0>=2.48)

if(GLIB_GOBJECT_FOUND)
    include_directories(${GLIB_GOBJECT_INCLUDE_DIRS})
    add_definitions(${GLIB_GOBJECT_CFLAGS} ${GLIB_GOBJECT_CFLAGS_OTHER})

    message( STATUS "GLIB_GOBJECT_LIBRARIES: " ${GLIB_GOBJECT_LIBRARIES} )
    message( STATUS "GLIB_GOBJECT_LIBRARY_DIRS: " ${GLIB_GOBJECT_LIBRARY_DIRS} )
    message( STATUS "GLIB_GOBJECT_LDFLAGS: " ${GLIB_GOBJECT_LDFLAGS} )
    message( STATUS "GLIB_GOBJECT_LDFLAGS_OTHER: " ${GLIB_GOBJECT_LDFLAGS_OTHER} )
    message( STATUS "GLIB_GOBJECT_INCLUDE_DIRS: " ${GLIB_GOBJECT_INCLUDE_DIRS} )
    message( STATUS "GLIB_GOBJECT_CFLAGS: " ${GLIB_GOBJECT_CFLAGS} )
    message( STATUS "GLIB_GOBJECT_CFLAGS_OTHER: " ${GLIB_GOBJECT_CFLAGS_OTHER} )
else()
    message( FATAL_ERROR "GLib & GObject >= 2.48 are required, exiting." )
endif()

# compiler-dependent options
if("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-Weverything)
    # linker (LLD)
    link_libraries("-fuse-ld=lld")
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-Wall -Wextra)
    # linker (gold)
    link_libraries("-fuse-ld=gold")
else()
    message( FATAL_ERROR "Only Clang and GCC are supported, exiting." )
endif()

# common options
add_compile_options(-pedantic)

# components
add_subdirectory(regexperience_core)
add_subdirectory(regexperience_cli)

# compiler being used
message( STATUS "CMAKE_C_COMPILER: " ${CMAKE_C_COMPILER} )

# build type
message( STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE} )
