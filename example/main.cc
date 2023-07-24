#include <drogon/drogon.h>

#include <drogon_wrappers/restful_ctrl.hpp>
#include <user.hpp>

#define CTRL(T)                                                                                         \
struct ctrl : restful_ctrl<T>                                                                           \
{                                                                                                       \
    METHOD_LIST_BEGIN                                                                                   \
    ADD_METHOD_TO(restful_ctrl::getOne, std::string{"/"} + T::table() + "/{1}", Get, Options);          \
    ADD_METHOD_TO(restful_ctrl::updateOne, std::string{"/"} + T::table() + "/{1}", Put, Options);       \
    ADD_METHOD_TO(restful_ctrl::deleteOne, std::string{"/"} + T::table() + "/{1}", Delete, Options);    \
    ADD_METHOD_TO(restful_ctrl::get, std::string{"/"} + T::table(), Get, Options);                      \
    ADD_METHOD_TO(restful_ctrl::create, std::string{"/"} + T::table(), Post, Options);                  \
    /*ADD_METHOD_TO(restful_ctrl::update, std::string{"/"} + T::table(), Put, Options);*/               \
    METHOD_LIST_END                                                                                     \
};

CTRL(user);

int main() {
    //Set HTTP listener address and port
    //drogon::app().addListener("0.0.0.0",80);
    //Load config file
    drogon::app().loadConfigFile("config.json");
    //Run HTTP framework,the method will block in the internal event loop
    drogon::app().run();
    return 0;
}
