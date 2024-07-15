cmake_minimum_required(VERSION 3.30 FATAL_ERROR)

project(crudpp VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BOOST_VERSION 1.83)

set(CRUDPP_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "")
set(CRUDPP_HEADERS
    ${CRUDPP_SOURCE_DIR}/include/concat.hpp
    ${CRUDPP_SOURCE_DIR}/include/singleton.hpp
    ${CRUDPP_SOURCE_DIR}/include/function_traits.hpp
    ${CRUDPP_SOURCE_DIR}/include/member_function_traits.hpp
    ${CRUDPP_SOURCE_DIR}/include/crudpp/concepts/types.hpp
    ${CRUDPP_SOURCE_DIR}/include/crudpp/concepts/required.hpp
    ${CRUDPP_SOURCE_DIR}/include/crudpp/concepts/permissions.hpp
    ${CRUDPP_SOURCE_DIR}/include/crudpp/macros.hpp
    ${CRUDPP_SOURCE_DIR}/include/crudpp/type_traits.hpp
    ${CRUDPP_SOURCE_DIR}/include/crudpp/utils.hpp
    CACHE INTERNAL ""
)
add_custom_target(crudpp SOURCES ${CRUDPP_HEADERS})
set(BINDING_HEADERS ${CRUDPP_SOURCE_DIR}/include/crudpp/bindings CACHE INTERNAL "")

function(make_all)
    option(PFR_PATH "Path to the independent Boost::pfr repository (boostorg/pfr)")
    if(IS_DIRECTORY ${PFR_PATH})
        include_directories(${PFR_PATH}/include)
    else()
        find_package(Boost ${BOOST_VERSION} COMPONENTS pfr)
    endif()

    include_directories(${CRUDPP_SOURCE_DIR}/include)

    set(ONE_VALUE_ARGS INCLUDE CLIENT_RESOURCE_DIR)

    set(MULTI_VALUE_ARGS CLASSES)
    cmake_parse_arguments(CRUDPP "" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})

    if("user" IN_LIST CRUDPP_CLASSES)
        set(USER_CLASS TRUE)
        add_compile_definitions(USER_CLASS=true)
    else()
        set(USER_CLASS FALSE)
        add_compile_definitions(USER_CLASS=false)
    endif()

    add_compile_definitions(INCLUDE=${CRUDPP_INCLUDE})

    find_package(Drogon CONFIG)
    if(${Drogon_FOUND} AND ${Boost_FOUND})
        # Boost preprocessor is also required for the Drogon binding
        # Only add if Boost is FOUND
        find_package(Boost ${BOOST_VERSION} COMPONENTS preprocessor)
        add_subdirectory(${CRUDPP_SOURCE_DIR}/src/bindings/drogon
            ${CMAKE_BINARY_DIR}/drogon)
    endif()

    find_package(Qt6 COMPONENTS Quick REQUIRED)
    if(${Qt6_FOUND})
        add_subdirectory(${CRUDPP_SOURCE_DIR}/src/bindings/qt
            ${CMAKE_BINARY_DIR}/qt)
    endif()

endfunction()

# Copied from https://github.com/celtera/avendish/blob/main/CMakeLists.txt#L18
# Only build examples if we are building this repo directly,
# not if it is add_subdirectory'd
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    add_subdirectory(example)
endif()

