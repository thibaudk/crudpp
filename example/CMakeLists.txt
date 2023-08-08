project(example CXX)

set(SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/user.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/carts_products.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ctrl_macro.cpp)

make_drogon_server()
