#pragma once

#include <crudpp/bindigs/drogon/wrappers/restful_ctrl.hpp>

template <authenticates T>
struct restful_ctrl<T, true, true> : public restful_ctrl<T, true>
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

        const auto identifier{object.*T::identifier()};

        auto dbClientPtr = this->getDbClient();
        auto callbackPtr =
            std::make_shared<std::function<void(const HttpResponsePtr &)>>(
                std::move(callback));
        drogon::orm::Mapper<model<T>> mapper(dbClientPtr);
        mapper.findByOne(
            Criteria{identifier.c_name(), CompareOperator::EQ, identifier.value},
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
};
