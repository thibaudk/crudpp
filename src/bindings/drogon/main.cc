#include <drogon/drogon.h>
#include <crudpp/macros.hpp>
#include <crudpp/bindings/drogon/ctrl_macro.hpp>

#include STRINGIFY_MACRO(INCLUDE)

MAKE_CTRLS(BOOST_FORMATED_CLASSES)

int main()
{
    //Load config file
    drogon::app().loadConfigFile("config.json");
    //Run HTTP framework,the method will block in the internal event loop
    drogon::app().run();
    return 0;
}
