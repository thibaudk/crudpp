cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(crudpp VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost QUIET REQUIRED)

set(CRUDPP_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(CRUDPP_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "")

include_directories(${CRUDPP_SOURCE_DIR}/include)

function(make_drogon_server)
    add_subdirectory(${CRUDPP_SOURCE_DIR}/src/bindings/drogon
        ${CMAKE_BINARY_DIR}/drogon)
endfunction(make_drogon_server)

function(make_qt_client)
    add_subdirectory(${CRUDPP_SOURCE_DIR}/src/bindings/qt
        ${CMAKE_BINARY_DIR}/qt)
endfunction(make_qt_client)

# Copied from https://github.com/celtera/avendish/blob/main/CMakeLists.txt#L18
# Only build examples if we are building this repo directly,
# not if it is add_subdirectory'd
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    add_subdirectory(example)
endif()
