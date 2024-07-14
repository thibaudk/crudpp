#pragma once

#include <drogon/HttpController.h>

#include <crudpp/bindings/drogon/wrappers/restful_ctrl_w_spk.hpp>

#include <crudpp/macros.hpp>
#include STRINGIFY_MACRO(INCLUDE)

using namespace drogon;

struct user_ctrl final : public HttpController<user_ctrl>
                       , private restful_ctrl<user, true>
{
        METHOD_LIST_BEGIN
        ADD_METHOD_TO(user_ctrl::create, std::string{"/"} + user::table(), Post);
        ADD_METHOD_TO(user_ctrl::get, std::string{"/"} + user::table(), Get, Options);
        ADD_METHOD_TO(user_ctrl::update_by, std::string{"/"} + user::table(), Put, Options);
        ADD_METHOD_TO(user_ctrl::delete_by, std::string{"/"} + user::table(), Delete, Options);
        ADD_METHOD_TO(user_ctrl::get_one, std::string{"/"} + user::table() + "/{}", Get);
        ADD_METHOD_TO(user_ctrl::update_one, std::string{"/"} + user::table() + "/{}", Put);
        ADD_METHOD_TO(user_ctrl::delete_one, std::string{"/"} + user::table() + "/{}", Delete);
        ADD_METHOD_TO(user_ctrl::auth, std::string{"/"} + user::table() + "/auth", Post);
        METHOD_LIST_END

        void auth(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr &)>&& callback);
};
