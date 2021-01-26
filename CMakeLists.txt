cmake_minimum_required(VERSION 3.1)

project("vdeplug_n2n")
include(GNUInstallDirs)
include(CheckIncludeFile)
include(FetchContent)
FetchContent_Declare(
	n2n
	GIT_REPOSITORY https://github.com/ntop/n2n.git
	GIT_TAG 2.8
)

FetchContent_MakeAvailable(n2n)
set(n2n ENABLE_EXPORTS ON)

include_directories("./build/_deps/n2n-src/include")
set(CMAKE_REQUIRED_QUIET TRUE)
set(LIBS_REQUIRED vdeplug_mod)
set(CMAKE_C_FLAGS "-Wall -Wshadow -Wpointer-arith -Wmissing-declarations -Wnested-externs -fPIC")

foreach(THISLIB IN LISTS LIBS_REQUIRED)
  find_library(LIB${THISLIB}_OK ${THISLIB})
  if(NOT LIB${THISLIB}_OK)
    message(FATAL_ERROR "library lib${THISLIB} not found")
  endif()
endforeach(THISLIB)

add_definitions(-D_GNU_SOURCE)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(vdeplug_n2nEdge SHARED libvdeplug_n2nEdge.c)
add_library(vdeplug_n2n_super SHARED libvdeplug_n2n_super.c)

target_link_libraries(vdeplug_n2nEdge n2n vdeplug_mod)
target_link_libraries(vdeplug_n2n_super n2n vdeplug_mod)

set_property(TARGET n2n PROPERTY POSITION_INDEPENDENT_CODE ON)


install(TARGETS vdeplug_n2nEdge vdeplug_n2n_super DESTINATION ${CMAKE_INSTALL_LIBDIR}/vdeplug)

add_custom_target(configure
	COMMAND /bin/sh "./autogen.sh"
	WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/build/_deps/n2n-src/"
)