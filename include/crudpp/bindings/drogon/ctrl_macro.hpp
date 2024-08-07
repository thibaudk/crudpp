#pragma once

#include <boost/preprocessor/seq/for_each.hpp>

#include <drogon/HttpController.h>

#include <crudpp/bindings/drogon/wrappers/restful_ctrl_w_spk.hpp>
#include <crudpp/bindings/drogon/wrappers/restful_ctrl.hpp>
#include <crudpp/concepts/required.hpp>

#define CTRL(T)                                                                            \
struct T##_ctrl final : public drogon::HttpController<T##_ctrl>                            \
                      , private restful_ctrl<T, crudpp::r_single_primary_key<T>>           \
{                                                                                          \
    METHOD_LIST_BEGIN                                                                      \
    ADD_METHOD_TO(T##_ctrl::create, std::string{"/"} + T::table(), Post);                  \
    ADD_METHOD_TO(T##_ctrl::get, std::string{"/"} + T::table(), Get, Options);             \
    ADD_METHOD_TO(T##_ctrl::update_by, std::string{"/"} + T::table(), Put, Options);       \
    ADD_METHOD_TO(T##_ctrl::delete_by, std::string{"/"} + T::table(), Delete, Options);    \
    ADD_METHOD_TO(T##_ctrl::get_one, std::string{"/"} + T::table() + "/{}", Get);          \
    ADD_METHOD_TO(T##_ctrl::update_one, std::string{"/"} + T::table() + "/{}", Put);       \
    ADD_METHOD_TO(T##_ctrl::delete_one, std::string{"/"} + T::table() + "/{}", Delete);    \
    METHOD_LIST_END                                                                        \
};

//#include <boost/preprocessor/control/if.hpp>

//#define CTRL_NO_PK(T)                                                                    \
//struct T##_ctrl : public drogon::HttpController<T##_ctrl>                                \
//                , public restful_ctrl<T, crudpp::has_primary_key<T>>                     \
//{                                                                                        \
//        METHOD_LIST_BEGIN                                                                \
//        ADD_METHOD_TO(T##_ctrl::get, std::string{"/"} + T::table(), Get, Options);       \
//        ADD_METHOD_TO(T##_ctrl::create, std::string{"/"} + T::table(), Post, Options);   \
//        METHOD_LIST_END                                                                  \
//};

//#define CTRLS_FROM_MACRO(r, data, elem)                                                  \
BOOST_PP_IF(crudpp::has_primary_key<T>, CTRL(elem), CTRL_NO_PK(elem))

#define CTRLS_FROM_MACRO(r, data, elem) CTRL(elem)

#define MAKE_CTRLS(CLASSES) BOOST_PP_SEQ_FOR_EACH(CTRLS_FROM_MACRO, , CLASSES)
