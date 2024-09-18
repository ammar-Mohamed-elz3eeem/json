# CMakeLists.txt for JSONTests
#
# © 2018 by Richard Walters

cmake_minimum_required(VERSION 3.8)
set(This JSONTests)

set(Sources
    src/JSONTests.cpp
)

add_executable(${This} ${Sources})
set_target_properties(${This} PROPERTIES
    FOLDER Tests
)

find_package(gtest REQUIRED)

target_include_directories(${This} PRIVATE ..)

target_link_libraries(${This} PUBLIC
    gtest_main
    JSON
    SystemAbstractions
    StringExtensions
)

add_test(
    NAME ${This}
    COMMAND ${This}
)
