cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(crudpp VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost QUIET REQUIRED)

#set(SRC
#    ${CMAKE_CURRENT_SOURCE_DIR}/include/concepts/required.hpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/include/drogon_wrappers/model.hpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/include/drogon_wrappers/restful_ctrl_base.hpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/include/drogon_wrappers/restful_ctrl.hpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/include/drogon_wrappers/ctrl_macro.hpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/include/drogon_visitors/row_handler.hpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/include/drogon_visitors/row_reader.hpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/include/drogon_visitors/json_handler.hpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/include/drogon_visitors/json_handler_omit_primary.hpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/include/drogon_visitors/json_reader.hpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/include/drogon_visitors/m_vector_handler.hpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/include/drogon_visitors/m_vector_handler_omit_primary.hpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/include/drogon_visitors/m_vector_reader.hpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Copied from https://github.com/celtera/avendish/blob/main/CMakeLists.txt#L18
# Only build examples if we are building this repo directly,
# not if it is add_subdirectory'd
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    add_subdirectory(example)
endif()
