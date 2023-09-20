#include <drogon/HttpController.h>
#include <crudpp/bindigs/drogon/wrappers/restful_ctrl.hpp>
#include <crudpp/utils.hpp>

using namespace drogon;

#define CTRL(T)                                                                                  \
struct T##_ctrl : public drogon::HttpController<T##_ctrl>                                        \
                , public restful_ctrl<T, crudpp::has_primary_key<T>>                             \
{                                                                                                \
    METHOD_LIST_BEGIN                                                                            \
    ADD_METHOD_TO(T##_ctrl::getOne, std::string{"/"} + T::table() + "/{}", Get, Options);        \
    ADD_METHOD_TO(T##_ctrl::updateOne, std::string{"/"} + T::table() + "/{}", Put, Options);     \
    ADD_METHOD_TO(T##_ctrl::deleteOne, std::string{"/"} + T::table() + "/{}", Delete, Options);  \
    ADD_METHOD_TO(T##_ctrl::get, std::string{"/"} + T::table(), Get, Options);                   \
    ADD_METHOD_TO(T##_ctrl::create, std::string{"/"} + T::table(), Post, Options);               \
    METHOD_LIST_END                                                                              \
};

#define CTRLS_FROM_MACRO(r, data, elem) CTRL(elem)
