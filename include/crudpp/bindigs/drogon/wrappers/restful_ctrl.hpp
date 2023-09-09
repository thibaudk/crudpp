#include <crudpp/bindigs/drogon/wrappers/restful_ctrl_base.hpp>

template <typename T>
struct restful_ctrl : public restful_ctrl_base<T>
{
    void getOne(const HttpRequestPtr &req,
                std::function<void(const HttpResponsePtr &)> &&callback,
                typename model<T>::PrimaryKeyType &&id)
    {
        auto dbClientPtr = this->getDbClient();
        auto callbackPtr =
            std::make_shared<std::function<void(const HttpResponsePtr &)>>(
                std::move(callback));
        drogon::orm::Mapper<model<T>> mapper(dbClientPtr);
        mapper.findByPrimaryKey(
            id,
            [req, callbackPtr, this](model<T> r) {
                (*callbackPtr)(HttpResponse::newHttpJsonResponse(this->makeJson(req, r)));
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
        if(!this->doCustomValidations(*jsonPtr, err))
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
            // workaroud validateMasqueradedJsonForUpdate
            if (!model<T>::validateJsonForUpdate(*jsonPtr, err))
            {
                Json::Value ret;
                ret["error"] = err;
                auto resp= HttpResponse::newHttpJsonResponse(ret);
                resp->setStatusCode(k400BadRequest);
                callback(resp);
                return;
            }

            if (this->isMasquerading())
                object.updateByMasqueradedJson(*jsonPtr, this->masqueradingVector());
            else
                object.updateByJson(*jsonPtr);
        }
        catch (const Json::Exception &e)
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

        auto dbClientPtr = this->getDbClient();
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
//                    TODO : reinstate as an error when redundant client requests can be prevented
//                    resp->setStatusCode(k404NotFound);
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

    void create(const HttpRequestPtr &req,
                std::function<void(const HttpResponsePtr &)> &&callback)
    {
        auto jsonPtr=req->jsonObject();
        if (jsonPtr)
        {
            std::string err;

            if (!this->doCustomValidations(*jsonPtr, err))
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
                (this->isMasquerading()?
                                   model<T>(*jsonPtr, this->masqueradingVector()) :
                     model<T>(*jsonPtr));
            auto dbClientPtr = this->getDbClient();
            auto callbackPtr =
                std::make_shared<std::function<void(const HttpResponsePtr &)>>(
                    std::move(callback));
            drogon::orm::Mapper<model<T>> mapper(dbClientPtr);
            mapper.insert(
                object,
                [req, callbackPtr, this](model<T> newObject){
                    (*callbackPtr)(HttpResponse::newHttpJsonResponse(
                        this->makeJson(req, newObject)));
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

    void deleteOne(const HttpRequestPtr &req,
                   std::function<void(const HttpResponsePtr &)> &&callback,
                   typename model<T>::PrimaryKeyType &&id)
    {
        auto dbClientPtr = this->getDbClient();
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
};
