# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText : 2022-2024 Uni-Libraries contributors

# catch: prefer an installed Catch2 3; fall back to CPM. find_package() must run in this directory scope,
# otherwise the CMAKE_MODULE_PATH entry for Catch.cmake added by Catch2Config.cmake is lost.
if(NOT TARGET Catch2::Catch2WithMain)
    find_package(Catch2 3 CONFIG QUIET)
    if(NOT Catch2_FOUND)
        include(../cmake/cpm.cmake)
        include(../cmake/cpm-catch.cmake)
        list(APPEND CMAKE_MODULE_PATH "${Catch2_SOURCE_DIR}/extras")
    endif()
endif()
include(Catch)

# test
add_executable(uni_ax25_test)
target_sources(uni_ax25_test PRIVATE uni_ax25_tests.cpp)
target_compile_features(uni_ax25_test PRIVATE cxx_std_17)
target_link_libraries(uni_ax25_test PRIVATE uni_ax25)
target_link_libraries(uni_ax25_test PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(uni_ax25_test)
