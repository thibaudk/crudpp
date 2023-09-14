project(example CXX)

make_all(
    INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/example.hpp
    CASSES
        user
        carts_products
    )
