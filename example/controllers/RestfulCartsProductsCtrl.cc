/**
 *
 *  RestfulCartsProductsCtrl.cc
 *  This file is generated by drogon_ctl
 *
 */

#include "RestfulCartsProductsCtrl.h"
#include <string>



void RestfulCartsProductsCtrl::get(const HttpRequestPtr &req,
                                   std::function<void(const HttpResponsePtr &)> &&callback)
{
    RestfulCartsProductsCtrlBase::get(req, std::move(callback));
}

void RestfulCartsProductsCtrl::create(const HttpRequestPtr &req,
                                      std::function<void(const HttpResponsePtr &)> &&callback)
{
    RestfulCartsProductsCtrlBase::create(req, std::move(callback));
}
