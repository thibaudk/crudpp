#pragma once

#include <crudpp/bindigs/drogon/wrappers/restful_ctrl_base.hpp>

// no primary key and no authentication
template <typename T, bool has_primary_key = false, bool authenticating = false>
struct restful_ctrl : public restful_ctrl_base<T>
{
    virtual void auth(const HttpRequestPtr &req,
                      std::function<void(const HttpResponsePtr &)> &&callback)
    {
        auto callbackPtr =
            std::make_shared<std::function<void(const HttpResponsePtr &)>>(
                std::move(callback));

        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k404NotFound);
        (*callbackPtr)(resp);
    }

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

// primary key and no authentication
template <typename T>
struct restful_ctrl<T, true, false> : public restful_ctrl_base<T>
{
    virtual void auth(const HttpRequestPtr &req,
                      std::function<void(const HttpResponsePtr &)> &&callback)
    {
        auto callbackPtr =
            std::make_shared<std::function<void(const HttpResponsePtr &)>>(
                std::move(callback));

        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k404NotFound);
        (*callbackPtr)(resp);
    }

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
            [callbackPtr, this](const DrogonDbException &e) {
                const drogon::orm::UnexpectedRows *s=dynamic_cast<const drogon::orm::UnexpectedRows *>(&e.base());
                if(s)
                {
                    auto resp = HttpResponse::newHttpResponse();
                    resp->setStatusCode(k404NotFound);
                    (*callbackPtr)(resp);
                    return;
                }

                this->internal_error(e, callbackPtr);
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
            [callbackPtr, this](const size_t count)
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
                    this->internal_error(callbackPtr);
                }
            },
            [callbackPtr, this](const DrogonDbException &e) { this->internal_error(e, callbackPtr); });
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
            [callbackPtr, this](const size_t count) {
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
                    this->internal_error(callbackPtr);
                }
            },
            [callbackPtr, this](const DrogonDbException &e) { this->internal_error(e, callbackPtr); });
    }
};

#include <crudpp/bindigs/drogon/visitors/json_handler.hpp>

// authenticating
template <typename T>
struct restful_ctrl<T, true, true> : public restful_ctrl<T, true, false>
{
    void auth(const HttpRequestPtr &req,
              std::function<void(const HttpResponsePtr &)> &&callback) override
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

        T tmp{};
        auto& unmae{tmp.username};
        auto& pwd{tmp.password};
        bool dirtyFlag_[2] = { false };
        json_handler handler{dirtyFlag_, *jsonPtr};

        handler(unmae);
        handler(pwd);

        if(!dirtyFlag_[0] || !dirtyFlag_[1])
        {
            Json::Value ret;
            ret["error"] = "missing username and/or password in the request";
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
        mapper.findOne(
            Criteria{unmae.c_name(), CompareOperator::EQ, unmae.value},
            [callbackPtr, req, &pwd, &mapper, this](model<T> r)
            {
                auto conf{HttpAppFramework::instance().getCustomConfig()["encryption"]};

                using namespace utils;
                auto (*hash)(const std::string&) = getSha256;

                if (conf.isMember("hash") && conf["hash"].isString())
                {
                    std::string h_str{conf["hash"].asString()};

                    if (h_str != "sha256")
                    {
                        if (h_str == "md5")
                            hash = getMd5;
                        else if (h_str == "sha1")
                            hash = getSha1;
                        else if (h_str == "sha3")
                            hash = getSha3;
                        else if (h_str == "blake2")
                            hash = getBlake2b;
                    }
                }

                std::string salt{};

                if (conf.isMember("salt") && conf["salt"].isString())
                    salt = conf["salt"].asString();

                int iterations{1}, i{0};

                if (conf.isMember("iterations") && conf["iterations"].isInt())
                    iterations = conf["iterations"].asInt();

                if (!salt.empty())
                {
                    pwd.value = hash(pwd.value + salt);
                    i++;
                }

                for (; i < iterations; i++)
                    pwd.value = hash(pwd.value);

                if (pwd.value != r.get_aggregate().password.value)
                {
                    Json::Value ret;
                    ret["error"] = "incorrect credentials";
                    auto resp= HttpResponse::newHttpJsonResponse(ret);
                    resp->setStatusCode(k401Unauthorized);
                    (*callbackPtr)(resp);
                    return;
                }

                r.template update<decltype(T::session_id)>(req->session()->sessionId());

                mapper.update(r,
                    [callbackPtr, req, r, this](const size_t)
                    { (*callbackPtr)(HttpResponse::newHttpJsonResponse(this->makeJson(req, r))); },
                    [callbackPtr, this](const DrogonDbException &e) { this->internal_error(e, callbackPtr); });
            },
            [callbackPtr, this](const DrogonDbException &e) {
                const drogon::orm::UnexpectedRows *s=dynamic_cast<const drogon::orm::UnexpectedRows *>(&e.base());
                if(s)
                {
                    Json::Value ret;
                    ret["error"] = "incorrect credentials";
                    auto resp= HttpResponse::newHttpJsonResponse(ret);
                    resp->setStatusCode(k401Unauthorized);
                    (*callbackPtr)(resp);
                    return;
                }

                this->internal_error(e, callbackPtr);
            });
    }
};
