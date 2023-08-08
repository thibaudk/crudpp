#include <drogon/HttpController.h>
#include <bindigs/drogon/wrappers/restful_ctrl.hpp>

using namespace drogon;

//template <typename T>
//struct ctrl : public drogon::HttpController<ctrl<T>>
//            , restful_ctrl<T>
//{
//    METHOD_LIST_BEGIN
//    ADD_METHOD_TO(ctrl<T>::getOne, std::string{"/"} + T::table() + "/{1}", Get, Options);
//    ADD_METHOD_TO(restful_ctrl<T>::updateOne, std::string{"/"} + T::table() + "/{1}", Put, Options);
//    ADD_METHOD_TO(restful_ctrl<T>::deleteOne, std::string{"/"} + T::table() + "/{1}", Delete, Options);
//    ADD_METHOD_TO(restful_ctrl<T>::get, std::string{"/"} + T::table(), Get, Options);
//    ADD_METHOD_TO(restful_ctrl<T>::create, std::string{"/"} + T::table(), Post, Options);
//    /*ADD_METHOD_TO(restful_ctrl::update, std::string{"/"} + T::table(), Put, Options);*/
//    METHOD_LIST_END
//};

#define CTRL(T)                                                                                  \
struct T##_ctrl : public drogon::HttpController<T##_ctrl>                                        \
                , restful_ctrl<T>                                                                \
{                                                                                                \
    METHOD_LIST_BEGIN                                                                            \
    ADD_METHOD_TO(T##_ctrl::getOne, std::string{"/"} + T::table() + "/{1}", Get, Options);       \
    ADD_METHOD_TO(T##_ctrl::updateOne, std::string{"/"} + T::table() + "/{1}", Put, Options);    \
    ADD_METHOD_TO(T##_ctrl::deleteOne, std::string{"/"} + T::table() + "/{1}", Delete, Options); \
    ADD_METHOD_TO(T##_ctrl::get, std::string{"/"} + T::table(), Get, Options);                   \
    ADD_METHOD_TO(T##_ctrl::create, std::string{"/"} + T::table(), Post, Options);               \
    /*ADD_METHOD_TO(T##_ctrl::update, std::string{"/"} + T::table(), Put, Options);*/            \
    METHOD_LIST_END                                                                              \
};

#define BASE_CTRL(T)                                                                             \
struct T##_ctrl : public drogon::HttpController<T##_ctrl>                                        \
                , restful_ctrl_base<T>                                                           \
{                                                                                                \
    METHOD_LIST_BEGIN                                                                        \
    ADD_METHOD_TO(T##_ctrl::get, std::string{"/"} + T::table(), Get, Options);               \
    ADD_METHOD_TO(T##_ctrl::create, std::string{"/"} + T::table(), Post, Options);           \
    /*ADD_METHOD_TO(T##_ctrl::update, std::string{"/"} + T::table(), Put, Options);*/        \
    METHOD_LIST_END                                                                          \
};
