#include <iostream>
#include <fstream>
#include <json/json.h>

#include <drogon/drogon.h>
#include <crudpp/macros.hpp>
#include <crudpp/bindings/drogon/ctrl_macro.hpp>

#define DEFAULT_TIMEOUT 1200

#include STRINGIFY_MACRO(INCLUDE)

MAKE_CTRLS(BOOST_FORMATED_CLASSES)

struct agg
{
    static consteval auto primary_key() { return &agg::a; }
    // struct {} a;
    int a;
    bool b;
};

template<typename T = agg>
void f() {}

// template<typename T = agg>
// template<agg>
// template<auto agg::*b>
// template<auto agg::*a>
    // template<>
// void f<agg>() {}

int main()
{
    const bool bb{};
    agg i_a{};

    // bool agg::*ptr{&agg::b};

    // using a_t = static_function_traits<decltype(f<nullptr>)>::param_type;
    // a_t ap{};

    // f<>();
    // f<i_a>();
    // f<&i_a.b>();
    // f<i_a.*ptr>();
    // f<&i_a::a>();

    // Load JSON config file
    Json::Value json;
    std::ifstream ifs;
    ifs.open("config.json");

    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;

    if (!Json::parseFromStream(builder, ifs, &json, &errs))
    {
        std::cerr << errs << std::endl;
        return EXIT_FAILURE;
    }

    // force enable session
    if (!json.isMember("app"))
    {
        const auto conf{json["app"]};

        if (conf.isMember("enable_session"))
        {
            if (!conf["enable_session"])
                drogon::app().enableSession(DEFAULT_TIMEOUT);
        }
        else
            drogon::app().enableSession(DEFAULT_TIMEOUT);
    }
    else
        drogon::app().enableSession(DEFAULT_TIMEOUT);

    drogon::app().loadConfigJson(json);
    drogon::app().run();
    return 0;
}
