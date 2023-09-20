project(example CXX)

make_all(
    INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/example.hpp
    USER_CLASS user
    CLASSES
        carts_products
    )
