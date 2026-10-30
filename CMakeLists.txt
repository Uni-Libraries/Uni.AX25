# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText : 2022-2024 Uni-Libraries contributors

#
# CMake
#

# 3.21: PROJECT_IS_TOP_LEVEL and C_STANDARD 23
cmake_minimum_required(VERSION 3.21)

project(uni_ax25 LANGUAGES C)


#
# Options
#

option(UNI_AX25_BUILD_TESTS "Build the Uni.AX25 tests" ${PROJECT_IS_TOP_LEVEL})
option(UNI_AX25_INSTALL "Generate the Uni.AX25 install rules" ${PROJECT_IS_TOP_LEVEL})


#
# Library
#

add_library(uni_ax25)
add_library(uni_ax25::uni_ax25 ALIAS uni_ax25)

target_sources(uni_ax25 PRIVATE
    src/uni_ax25.c
    src/uni_ax25_hdlc.c
)

target_include_directories(uni_ax25 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(MSVC)
    set_target_properties(uni_ax25 PROPERTIES C_STANDARD 17)
    target_compile_options(uni_ax25 PRIVATE /W4)
else()
    set_target_properties(uni_ax25 PROPERTIES C_STANDARD 23)
    target_compile_options(uni_ax25 PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(uni_ax25 PROPERTIES
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
    # the static library is meant to be linked into shared objects as well
    POSITION_INDEPENDENT_CODE ON
    # there are no export annotations; export everything when built as a DLL
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)


#
# Installation
#

if(UNI_AX25_INSTALL)
    install(
        TARGETS uni_ax25
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
    )
    install(FILES include/uni_ax25.h include/uni_ax25_hdlc.h DESTINATION include)
endif()


#
# Tests
#

if(UNI_AX25_BUILD_TESTS)
    enable_language(CXX)
    enable_testing()
    add_subdirectory(src_tests)
endif()
