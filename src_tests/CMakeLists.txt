# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText : 2022-2024 Uni-Libraries contributors

include(../cmake/cpm.cmake)
include(../cmake/cpm-catch.cmake)

# catch
list(APPEND CMAKE_MODULE_PATH "${Catch2_SOURCE_DIR}/extras")
include(Catch)

# test
add_executable(uni_ax25_test)
target_sources(uni_ax25_test PRIVATE uni_ax25_tests.cpp)
target_link_libraries(uni_ax25_test PRIVATE uni_ax25)
target_link_libraries(uni_ax25_test PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(uni_ax25_test)
