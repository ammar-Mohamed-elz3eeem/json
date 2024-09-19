
# CMakeLists.txt for JSON
#
# © 2018 by Richard Walters

cmake_minimum_required(VERSION 3.8)
set(This JSON)

set(Headers
    include/JSON/JSON.hpp
)

set(Sources
    src/JSON.cpp
)

add_library(${This} STATIC ${Sources} ${Headers})

set_target_properties(${This} PROPERTIES
    FOLDER Libraries
)

target_include_directories(${This} PUBLIC include)

target_link_libraries(${This} PUBLIC
    SystemAbstractions
    StringExtensions
    Utf8
)

add_subdirectory(test)
