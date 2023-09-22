#pragma once

#include <crudpp/bindigs/drogon/wrappers/restful_ctrl_base.hpp>

template <typename T, bool has_primary_key = false, bool authenticates = false>
struct restful_ctrl : public restful_ctrl_base<T>
{
    void getOne(const HttpRequestPtr &req,
                std::function<void(const HttpResponsePtr &)> &&callback)
    {
        auto callbackPtr =
            std::make_shared<std::function<void(const HttpResponsePtr &)>>(
                std::move(callback));

        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k404NotFound);
        (*callbackPtr)(resp);
    }

    void updateOne(const HttpRequestPtr &req,
                   std::function<void(const HttpResponsePtr &)> &&callback)
    {
        auto callbackPtr =
            std::make_shared<std::function<void(const HttpResponsePtr &)>>(
                std::move(callback));

        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k404NotFound);
        (*callbackPtr)(resp);
    }

    void deleteOne(const HttpRequestPtr &req,
                   std::function<void(const HttpResponsePtr &)> &&callback)
    {
        auto callbackPtr =
            std::make_shared<std::function<void(const HttpResponsePtr &)>>(
                std::move(callback));

        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k404NotFound);
        (*callbackPtr)(resp);
    }
};
