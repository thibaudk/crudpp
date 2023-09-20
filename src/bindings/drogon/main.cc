#include <boost/preprocessor/seq/for_each.hpp>
#include <drogon/drogon.h>
#include <crudpp/macros.hpp>

#include STRINGIFY_MACRO(INCLUDE)

BOOST_PP_SEQ_FOR_EACH(CTRLS_FROM_MACRO, , BOOST_FORMATED_CLASSES)

int main()
{
    //Set HTTP listener address and port
    //drogon::app().addListener("0.0.0.0",80);
    //Load config file

    drogon::app().loadConfigFile("config.json");
    //Run HTTP framework,the method will block in the internal event loop
    drogon::app().run();
    return 0;
}
