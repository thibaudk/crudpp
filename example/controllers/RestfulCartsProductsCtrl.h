/**
 *
 *  RestfulCartsProductsCtrl.h
 *  This file is generated by drogon_ctl
 *
 */

#pragma once

#include <drogon/HttpController.h>
#include "RestfulCartsProductsCtrlBase.h"

#include "CartsProducts.h"
using namespace drogon;
using namespace drogon_model::example;
/**
 * @brief this class is created by the drogon_ctl command.
 * this class is a restful API controller for reading and writing the carts_products table.
 */

class RestfulCartsProductsCtrl: public drogon::HttpController<RestfulCartsProductsCtrl>, public RestfulCartsProductsCtrlBase
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(RestfulCartsProductsCtrl::get,"/cartsproducts",Get,Options);
    ADD_METHOD_TO(RestfulCartsProductsCtrl::create,"/cartsproducts",Post,Options);
    //ADD_METHOD_TO(RestfulCartsProductsCtrl::update,"/cartsproducts",Put,Options);
    METHOD_LIST_END
     
    void get(const HttpRequestPtr &req,
             std::function<void(const HttpResponsePtr &)> &&callback);
    void create(const HttpRequestPtr &req,
             std::function<void(const HttpResponsePtr &)> &&callback);

};
