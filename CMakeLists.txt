# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText : 2022-2024 Uni-Libraries contributors

#
# CMake
#

cmake_minimum_required(VERSION 3.10)

project(uni_ax25)


#
# Library
#

add_library(uni_ax25)

target_sources(uni_ax25 PRIVATE
    src/uni_ax25.c
)

target_include_directories(uni_ax25 PUBLIC include)

if(MSVC)
    set_target_properties(uni_ax25 PROPERTIES C_STANDARD 17)
else()
    set_target_properties(uni_ax25 PROPERTIES C_STANDARD 23)
endif()



#
# Installation
#

install(
    TARGETS uni_ax25
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)



#
# Tests
#

if(PROJECT_IS_TOP_LEVEL)
    enable_testing()
    add_subdirectory(src_tests)
endif()
