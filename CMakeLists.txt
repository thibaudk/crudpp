cmake_minimum_required(VERSION 3.5)

project(crudpp VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(example ON CACHE BOOL "building example")

set(SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/concepts/required.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/drogon_wrappers/model.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/drogon_wrappers/restful_ctrl_base.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/drogon_wrappers/restful_ctrl.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/drogon_wrappers/ctrl_macro.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/drogon_visitors/row_handler.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/drogon_visitors/row_reader.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/drogon_visitors/json_handler.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/drogon_visitors/json_reader.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/drogon_visitors/m_vector_handler.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/drogon_visitors/m_vector_reader.hpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

if(example)
    add_subdirectory(example)
endif()
