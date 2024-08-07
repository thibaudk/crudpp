/**
 *
 *  RestfulReviewsCtrl.cc
 *  This file is generated by drogon_ctl
 *
 */

#include "RestfulReviewsCtrl.h"
#include <string>


void RestfulReviewsCtrl::getOne(const HttpRequestPtr &req,
                                std::function<void(const HttpResponsePtr &)> &&callback,
                                Reviews::PrimaryKeyType &&id)
{
    RestfulReviewsCtrlBase::getOne(req, std::move(callback), std::move(id));
}


void RestfulReviewsCtrl::updateOne(const HttpRequestPtr &req,
                                   std::function<void(const HttpResponsePtr &)> &&callback,
                                   Reviews::PrimaryKeyType &&id)
{
    RestfulReviewsCtrlBase::updateOne(req, std::move(callback), std::move(id));
}


void RestfulReviewsCtrl::deleteOne(const HttpRequestPtr &req,
                                   std::function<void(const HttpResponsePtr &)> &&callback,
                                   Reviews::PrimaryKeyType &&id)
{
    RestfulReviewsCtrlBase::deleteOne(req, std::move(callback), std::move(id));
}

void RestfulReviewsCtrl::get(const HttpRequestPtr &req,
                             std::function<void(const HttpResponsePtr &)> &&callback)
{
    RestfulReviewsCtrlBase::get(req, std::move(callback));
}

void RestfulReviewsCtrl::create(const HttpRequestPtr &req,
                                std::function<void(const HttpResponsePtr &)> &&callback)
{
    RestfulReviewsCtrlBase::create(req, std::move(callback));
}
