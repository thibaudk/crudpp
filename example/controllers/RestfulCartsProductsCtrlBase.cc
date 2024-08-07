
/**
 *
 *  RestfulCartsProductsCtrlBase.cc
 *  DO NOT EDIT. This file is generated by drogon_ctl automatically.
 *  Users should implement business logic in the derived class.
 */

#include "RestfulCartsProductsCtrlBase.h"
#include <string>


void RestfulCartsProductsCtrlBase::get(const HttpRequestPtr &req,
                                       std::function<void(const HttpResponsePtr &)> &&callback)
{
    auto dbClientPtr = getDbClient();
    drogon::orm::Mapper<CartsProducts> mapper(dbClientPtr);
    auto &parameters = req->parameters();
    auto iter = parameters.find("sort");
    if(iter != parameters.end())
    {
        auto sortFields = drogon::utils::splitString(iter->second, ",");
        for(auto &field : sortFields)
        {
            if(field.empty())
                continue;
            if(field[0] == '+')
            {
                field = field.substr(1);
                mapper.orderBy(field, SortOrder::ASC);
            }
            else if(field[0] == '-')
            {
                field = field.substr(1);
                mapper.orderBy(field, SortOrder::DESC);
            }
            else
            {
                mapper.orderBy(field, SortOrder::ASC);
            }
        }
    }
    iter = parameters.find("offset");
    if(iter != parameters.end())
    {
        try{
            auto offset = std::stoll(iter->second);
            mapper.offset(offset);
        }
        catch(...)
        {
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k400BadRequest);
            callback(resp);
            return;
        }
    }
    iter = parameters.find("limit");
    if(iter != parameters.end())
    {
        try{
            auto limit = std::stoll(iter->second);
            mapper.limit(limit);
        }
        catch(...)
        {
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k400BadRequest);
            callback(resp);
            return;
        }
    }    
    auto callbackPtr =
        std::make_shared<std::function<void(const HttpResponsePtr &)>>(
            std::move(callback));
    auto jsonPtr = req->jsonObject();
    if(jsonPtr && jsonPtr->isMember("filter"))
    {
        try{
            auto criteria = makeCriteria((*jsonPtr)["filter"]);
            mapper.findBy(criteria,
                [req, callbackPtr, this](const std::vector<CartsProducts> &v) {
                    Json::Value ret;
                    ret.resize(0);
                    for (auto &obj : v)
                    {
                        ret.append(makeJson(req, obj));
                    }
                    (*callbackPtr)(HttpResponse::newHttpJsonResponse(ret));
                },
                [callbackPtr](const DrogonDbException &e) { 
                    LOG_ERROR << e.base().what();
                    Json::Value ret;
                    ret["error"] = "database error";
                    auto resp = HttpResponse::newHttpJsonResponse(ret);
                    resp->setStatusCode(k500InternalServerError);
                    (*callbackPtr)(resp);    
                });
        }
        catch(const std::exception &e)
        {
            LOG_ERROR << e.what();
            Json::Value ret;
            ret["error"] = e.what();
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(k400BadRequest);
            (*callbackPtr)(resp);
            return;    
        }
    }
    else
    {
        mapper.findAll([req, callbackPtr, this](const std::vector<CartsProducts> &v) {
                Json::Value ret;
                ret.resize(0);
                for (auto &obj : v)
                {
                    ret.append(makeJson(req, obj));
                }
                (*callbackPtr)(HttpResponse::newHttpJsonResponse(ret));
            },
            [callbackPtr](const DrogonDbException &e) { 
                LOG_ERROR << e.base().what();
                Json::Value ret;
                ret["error"] = "database error";
                auto resp = HttpResponse::newHttpJsonResponse(ret);
                resp->setStatusCode(k500InternalServerError);
                (*callbackPtr)(resp);    
            });
    }
}

void RestfulCartsProductsCtrlBase::create(const HttpRequestPtr &req,
                                          std::function<void(const HttpResponsePtr &)> &&callback)
{
    auto jsonPtr=req->jsonObject();
    if(!jsonPtr)
    {
        Json::Value ret;
        ret["error"]="No json object is found in the request";
        auto resp= HttpResponse::newHttpJsonResponse(ret);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }
    std::string err;
    if(!doCustomValidations(*jsonPtr, err))
    {
        Json::Value ret;
        ret["error"] = err;
        auto resp= HttpResponse::newHttpJsonResponse(ret);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }
    if(isMasquerading())
    {
        if(!CartsProducts::validateMasqueradedJsonForCreation(*jsonPtr, masqueradingVector(), err))
        {
            Json::Value ret;
            ret["error"] = err;
            auto resp= HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(k400BadRequest);
            callback(resp);
            return;
        }
    }
    else
    {
        if(!CartsProducts::validateJsonForCreation(*jsonPtr, err))
        {
            Json::Value ret;
            ret["error"] = err;
            auto resp= HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(k400BadRequest);
            callback(resp);
            return;
        }
    }   
    try 
    {
        CartsProducts object = 
            (isMasquerading()? 
             CartsProducts(*jsonPtr, masqueradingVector()) : 
             CartsProducts(*jsonPtr));
        auto dbClientPtr = getDbClient();
        auto callbackPtr =
            std::make_shared<std::function<void(const HttpResponsePtr &)>>(
                std::move(callback));
        drogon::orm::Mapper<CartsProducts> mapper(dbClientPtr);
        mapper.insert(
            object,
            [req, callbackPtr, this](CartsProducts newObject){
                (*callbackPtr)(HttpResponse::newHttpJsonResponse(
                    makeJson(req, newObject)));
            },
            [callbackPtr](const DrogonDbException &e){
                LOG_ERROR << e.base().what();
                Json::Value ret;
                ret["error"] = "database error";
                auto resp = HttpResponse::newHttpJsonResponse(ret);
                resp->setStatusCode(k500InternalServerError);
                (*callbackPtr)(resp);   
            });
    }
    catch(const Json::Exception &e)
    {
        LOG_ERROR << e.what();
        Json::Value ret;
        ret["error"]="Field type error";
        auto resp= HttpResponse::newHttpJsonResponse(ret);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;        
    }   
}

/*
void RestfulCartsProductsCtrlBase::update(const HttpRequestPtr &req,
                                          std::function<void(const HttpResponsePtr &)> &&callback)
{

}*/

RestfulCartsProductsCtrlBase::RestfulCartsProductsCtrlBase()
    : RestfulController({
          "cart_id",
          "product_id"
      })
{
   /**
    * The items in the vector are aliases of column names in the table.
    * if one item is set to an empty string, the related column is not sent
    * to clients.
    */
    enableMasquerading({
        "cart_id", // the alias for the cart_id column.
        "product_id"  // the alias for the product_id column.
    });
}
