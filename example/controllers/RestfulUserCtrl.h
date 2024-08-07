/**
 *
 *  RestfulUserCtrl.h
 *  This file is generated by drogon_ctl
 *
 */

#pragma once

#include <drogon/HttpController.h>
#include "RestfulUserCtrlBase.h"

#include "User.h"
using namespace drogon;
using namespace drogon_model::example;
/**
 * @brief this class is created by the drogon_ctl command.
 * this class is a restful API controller for reading and writing the user table.
 */

class RestfulUserCtrl: public drogon::HttpController<RestfulUserCtrl>, public RestfulUserCtrlBase
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(RestfulUserCtrl::getOne,"/user/{1}",Get,Options);
    ADD_METHOD_TO(RestfulUserCtrl::updateOne,"/user/{1}",Put,Options);
    ADD_METHOD_TO(RestfulUserCtrl::deleteOne,"/user/{1}",Delete,Options);
    ADD_METHOD_TO(RestfulUserCtrl::get,"/user",Get,Options);
    ADD_METHOD_TO(RestfulUserCtrl::create,"/user",Post,Options);
    //ADD_METHOD_TO(RestfulUserCtrl::update,"/user",Put,Options);
    METHOD_LIST_END
     
    void getOne(const HttpRequestPtr &req,
                std::function<void(const HttpResponsePtr &)> &&callback,
                User::PrimaryKeyType &&id);
    void updateOne(const HttpRequestPtr &req,
                   std::function<void(const HttpResponsePtr &)> &&callback,
                   User::PrimaryKeyType &&id);
    void deleteOne(const HttpRequestPtr &req,
                   std::function<void(const HttpResponsePtr &)> &&callback,
                   User::PrimaryKeyType &&id);
    void get(const HttpRequestPtr &req,
             std::function<void(const HttpResponsePtr &)> &&callback);
    void create(const HttpRequestPtr &req,
             std::function<void(const HttpResponsePtr &)> &&callback);

};
