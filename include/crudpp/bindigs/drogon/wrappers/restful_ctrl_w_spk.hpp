#pragma once

#include <crudpp/bindigs/drogon/wrappers/restful_ctrl_base.hpp>

// single primary key and no authentication
template <typename T>
struct restful_ctrl<T, true, false> : public restful_ctrl_base<T>
{
    virtual void auth(const HttpRequestPtr &req,
                      std::function<void(const HttpResponsePtr &)> &&callback)
    {
        this->error(callback, k404NotFound);
    }

    void get_one(const HttpRequestPtr &req,
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
                const drogon::orm::UnexpectedRows *s=
                    dynamic_cast<const drogon::orm::UnexpectedRows *>(&e.base());
                if(s)
                {
                    this->error(callbackPtr, k404NotFound);
                    return;
                }

                this->internal_error(e, callbackPtr);
            });
    }

    void update_one(const HttpRequestPtr &req,
                   std::function<void(const HttpResponsePtr &)> &&callback,
                   typename model<T>::PrimaryKeyType &&id)
    {
        auto jsonPtr=req->jsonObject();
        if(!jsonPtr)
        {
            this->error(callback, "No json object is found in the request", k400BadRequest);
            return;
        }

        model<T> object;
        if (!this->read_json(callback, jsonPtr, object))
            return;

        if(object.getPrimaryKey() != id)
        {
            this->error(callback, "Bad primary key", k400BadRequest);
            return;
        }

        auto callbackPtr =
            std::make_shared<std::function<void(const HttpResponsePtr &)>>(
                std::move(callback));

        this->db_update(object, callbackPtr);
    }

    void delete_one(const HttpRequestPtr &req,
                   std::function<void(const HttpResponsePtr &)> &&callback,
                   typename model<T>::PrimaryKeyType &&id)
    {
        auto callbackPtr =
            std::make_shared<std::function<void(const HttpResponsePtr &)>>(
                std::move(callback));

        this->db_delete(id, callbackPtr);
    }
};
