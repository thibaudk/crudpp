#include <drogon/HttpController.h>
#include <drogon/orm/RestfulController.h>

#include <drogon_wrappers/model.hpp>

using namespace crudpp::wrapper;

using namespace drogon;
using namespace drogon::orm;

template <typename T>
class restful_ctrl : public drogon::HttpController<restful_ctrl<T>>
                   , public RestfulController
{
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(restful_ctrl::getOne, "/" + T::table + "/{1}", Get, Options);
    ADD_METHOD_TO(restful_ctrl::updateOne, "/" + T::table + "/{1}", Put, Options);
    ADD_METHOD_TO(restful_ctrl::deleteOne, "/" + T::table + "/{1}", Delete, Options);
    ADD_METHOD_TO(restful_ctrl::get, "/" + T::table, Get, Options);
    ADD_METHOD_TO(restful_ctrl::create, "/" + T::table, Post, Options);
    //ADD_METHOD_TO(restful_ctrl::update, "/" + T::table, Put, Options);
    METHOD_LIST_END

    void getOne(const HttpRequestPtr &req,
                std::function<void(const HttpResponsePtr &)> &&callback,
                typename model<T>::PrimaryKeyType &&id)
    {

        auto dbClientPtr = getDbClient();
        auto callbackPtr =
            std::make_shared<std::function<void(const HttpResponsePtr &)>>(
                std::move(callback));
        drogon::orm::Mapper<model<T>> mapper(dbClientPtr);
        mapper.findByPrimaryKey(
            id,
            [req, callbackPtr, this](model<T> r) {
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

    void updateOne(const HttpRequestPtr &req,
                   std::function<void(const HttpResponsePtr &)> &&callback,
                   typename model<T>::PrimaryKeyType &&id)
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
        model<T> object;
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
                if(!model<T>::validateMasqueradedJsonForUpdate(*jsonPtr, masqueradingVector(), err))
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
                if(!model<T>::validateJsonForUpdate(*jsonPtr, err))
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
        drogon::orm::Mapper<model<T>> mapper(dbClientPtr);

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

    void deleteOne(const HttpRequestPtr &req,
                   std::function<void(const HttpResponsePtr &)> &&callback,
                   typename model<T>::PrimaryKeyType &&id)
    {

        auto dbClientPtr = getDbClient();
        auto callbackPtr =
            std::make_shared<std::function<void(const HttpResponsePtr &)>>(
                std::move(callback));
        drogon::orm::Mapper<model<T>> mapper(dbClientPtr);
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

    void get(const HttpRequestPtr &req,
             std::function<void(const HttpResponsePtr &)> &&callback)
    {
        auto dbClientPtr = getDbClient();
        drogon::orm::Mapper<model<T>> mapper(dbClientPtr);
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
                    [req, callbackPtr, this](const std::vector<model<T>> &v) {
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
            mapper.findAll([req, callbackPtr, this](const std::vector<model<T>> &v) {
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

    void create(const HttpRequestPtr &req,
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
            if(!model<T>::validateMasqueradedJsonForCreation(*jsonPtr, masqueradingVector(), err))
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
            if(!model<T>::validateJsonForCreation(*jsonPtr, err))
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
            model<T> object =
                (isMasquerading()?
                     model<T>(*jsonPtr, masqueradingVector()) :
                     model<T>(*jsonPtr));
            auto dbClientPtr = getDbClient();
            auto callbackPtr =
                std::make_shared<std::function<void(const HttpResponsePtr &)>>(
                    std::move(callback));
            drogon::orm::Mapper<model<T>> mapper(dbClientPtr);
            mapper.insert(
                object,
                [req, callbackPtr, this](model<T> newObject){
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

//  void update(const HttpRequestPtr &req,
//              std::function<void(const HttpResponsePtr &)> &&callback);

    orm::DbClientPtr getDbClient() 
    {
        return drogon::app().getDbClient(dbClientName_);
    }

protected:
    /// Ensure that subclasses inherited from this class are instantiated.
    restful_ctrl()
        : RestfulController{ model<T>::insertColumns() }
    {
    /**
    * The items in the vector are aliases of column names in the table.
    * if one item is set to an empty string, the related column is not sent
    * to clients.
    */
        enableMasquerading(model<T>::insertColumns());
    }

    const std::string dbClientName_{"default"};
};
