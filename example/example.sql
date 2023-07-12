CREATE TABLE user 
(
    id INT AUTO_INCREMENT,
    PRIMARY KEY(id),
    username VARCHAR(63),
    password VARCHAR(63)
)

CREATE TABLE products 
(
    id INT AUTO_INCREMENT,
    PRIMARY KEY(id),
    product_name VARCHAR(63)
)

CREATE TABLE skus 
(
    unit CHAR(8) UNIQUE,
    product_id INT UNIQUE,
    CONSTRAINT FOREIGN KEY(product_id) REFERENCES products (id)
)

CREATE TABLE reviews 
(
    id INT AUTO_INCREMENT,
    PRIMARY KEY(id),
    text VARCHAR(255),
    product_id INT,
    CONSTRAINT FOREIGN KEY(product_id) REFERENCES products (id)
)

CREATE TABLE carts 
(
    id INT AUTO_INCREMENT,
    PRIMARY KEY(id),
    user_id INT UNIQUE,
    CONSTRAINT FOREIGN KEY(user_id) REFERENCES user (id)
)

CREATE TABLE carts_products 
(
    cart_id INT,
    CONSTRAINT FOREIGN KEY(cart_id) REFERENCES carts (id),
    product_id INT,
    CONSTRAINT FOREIGN KEY(product_id) REFERENCES products (id)
)