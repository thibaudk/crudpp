
/**
 *
 *  RestfulProductsCtrlBase.cc
 *  DO NOT EDIT. This file is generated by drogon_ctl automatically.
 *  Users should implement business logic in the derived class.
 */

#include "RestfulProductsCtrlBase.h"
#include <string>

void RestfulProductsCtrlBase::getOne(const HttpRequestPtr &req,
                                     std::function<void(const HttpResponsePtr &)> &&callback,
                                     Products::PrimaryKeyType &&id)
{

    auto dbClientPtr = getDbClient();
    auto callbackPtr =
        std::make_shared<std::function<void(const HttpResponsePtr &)>>(
            std::move(callback));
    drogon::orm::Mapper<Products> mapper(dbClientPtr);
    mapper.findByPrimaryKey(
        id,
        [req, callbackPtr, this](Products r) {
            (*callbackPtr)(HttpResponse::newHttpJsonResponse(makeJson(req, r)));
        },
        [callbackPtr](const DrogonDbException &e) {
            const drogon::orm::UnexpectedRows *s=dynamic_cast<const drogon::orm::UnexpectedRows *>(&e.base());
            if(s)
            {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k404NotFound);
                (*callbackPtr)(resp);
                return;
            }
            LOG_ERROR<<e.base().what();
            Json::Value ret;
            ret["error"] = "database error";
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(k500InternalServerError);
            (*callbackPtr)(resp);
        });
}


void RestfulProductsCtrlBase::updateOne(const HttpRequestPtr &req,
                                        std::function<void(const HttpResponsePtr &)> &&callback,
                                        Products::PrimaryKeyType &&id)
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
    Products object;
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
    try
    {
        if(isMasquerading())
        {
            if(!Products::validateMasqueradedJsonForUpdate(*jsonPtr, masqueradingVector(), err))
            {
                Json::Value ret;
                ret["error"] = err;
                auto resp= HttpResponse::newHttpJsonResponse(ret);
                resp->setStatusCode(k400BadRequest);
                callback(resp);
                return;
            }
            object.updateByMasqueradedJson(*jsonPtr, masqueradingVector());
        }
        else
        {
            if(!Products::validateJsonForUpdate(*jsonPtr, err))
            {
                Json::Value ret;
                ret["error"] = err;
                auto resp= HttpResponse::newHttpJsonResponse(ret);
                resp->setStatusCode(k400BadRequest);
                callback(resp);
                return;
            }
            object.updateByJson(*jsonPtr);
        }
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
    if(object.getPrimaryKey() != id)
    {
        Json::Value ret;
        ret["error"]="Bad primary key";
        auto resp= HttpResponse::newHttpJsonResponse(ret);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    auto dbClientPtr = getDbClient();
    auto callbackPtr =
        std::make_shared<std::function<void(const HttpResponsePtr &)>>(
            std::move(callback));
    drogon::orm::Mapper<Products> mapper(dbClientPtr);

    mapper.update(
        object,
        [callbackPtr](const size_t count) 
        {
            if(count == 1)
            {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k202Accepted);
                (*callbackPtr)(resp);
            }
            else if(count == 0)
            {
                Json::Value ret;
                ret["error"]="No resources are updated";
                auto resp = HttpResponse::newHttpJsonResponse(ret);
                resp->setStatusCode(k404NotFound);
                (*callbackPtr)(resp);
            }
            else
            {
                LOG_FATAL << "More than one resource is updated: " << count;
                Json::Value ret;
                ret["error"] = "database error";
                auto resp = HttpResponse::newHttpJsonResponse(ret);
                resp->setStatusCode(k500InternalServerError);
                (*callbackPtr)(resp);
            }
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


void RestfulProductsCtrlBase::deleteOne(const HttpRequestPtr &req,
                                        std::function<void(const HttpResponsePtr &)> &&callback,
                                        Products::PrimaryKeyType &&id)
{

    auto dbClientPtr = getDbClient();
    auto callbackPtr =
        std::make_shared<std::function<void(const HttpResponsePtr &)>>(
            std::move(callback));
    drogon::orm::Mapper<Products> mapper(dbClientPtr);
    mapper.deleteByPrimaryKey(
        id,
        [callbackPtr](const size_t count) {
            if(count == 1)
            {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k204NoContent);
                (*callbackPtr)(resp);
            }
            else if(count == 0)
            {
                Json::Value ret;
                ret["error"] = "No resources deleted";
                auto resp = HttpResponse::newHttpJsonResponse(ret);
                resp->setStatusCode(k404NotFound);
                (*callbackPtr)(resp);
            }
            else
            {
                LOG_FATAL << "Delete more than one records: " << count;
                Json::Value ret;
                ret["error"] = "Database error";
                auto resp = HttpResponse::newHttpJsonResponse(ret);
                resp->setStatusCode(k500InternalServerError);
                (*callbackPtr)(resp);
            }
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

void RestfulProductsCtrlBase::get(const HttpRequestPtr &req,
                                  std::function<void(const HttpResponsePtr &)> &&callback)
{
    auto dbClientPtr = getDbClient();
    drogon::orm::Mapper<Products> mapper(dbClientPtr);
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
                [req, callbackPtr, this](const std::vector<Products> &v) {
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
        mapper.findAll([req, callbackPtr, this](const std::vector<Products> &v) {
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

void RestfulProductsCtrlBase::create(const HttpRequestPtr &req,
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
        if(!Products::validateMasqueradedJsonForCreation(*jsonPtr, masqueradingVector(), err))
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
        if(!Products::validateJsonForCreation(*jsonPtr, err))
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
        Products object = 
            (isMasquerading()? 
             Products(*jsonPtr, masqueradingVector()) : 
             Products(*jsonPtr));
        auto dbClientPtr = getDbClient();
        auto callbackPtr =
            std::make_shared<std::function<void(const HttpResponsePtr &)>>(
                std::move(callback));
        drogon::orm::Mapper<Products> mapper(dbClientPtr);
        mapper.insert(
            object,
            [req, callbackPtr, this](Products newObject){
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
void RestfulProductsCtrlBase::update(const HttpRequestPtr &req,
                                     std::function<void(const HttpResponsePtr &)> &&callback)
{

}*/

RestfulProductsCtrlBase::RestfulProductsCtrlBase()
    : RestfulController({
          "id",
          "product_name"
      })
{
   /**
    * The items in the vector are aliases of column names in the table.
    * if one item is set to an empty string, the related column is not sent
    * to clients.
    */
    enableMasquerading({
        "id", // the alias for the id column.
        "product_name"  // the alias for the product_name column.
    });
}