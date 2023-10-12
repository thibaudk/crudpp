# crudpp

declarative three-tier architecture based on [avendish](https://github.com/celtera/avendish). \
Wright classes once, compile to a client app, and a server connected to database.

## Dependecies

* C++ 20 compiler
* CMake >= 3.13
* [Boost.PFR](https://github.com/boostorg/pfr)
* [Boost.preprocessor](https://github.com/boostorg/preprocessor)
* [Drogon Framework](https://github.com/drogonframework/drogon): currently the only suported server binding
* [Qt](https://github.com/drogonframework/drogon) & [Verdigris](https://github.com/woboq/verdigris): currently the only suported client binding

## Developement state
Only MariaDb is curently suported, and all tables need to be created manually. \
All QML files need to be written manually and provided as EXTRA_CLIENT_SOURCES in cmake. \
See the advancement of the project [here](https://github.com/users/thibaudk/projects/3/views/1).
