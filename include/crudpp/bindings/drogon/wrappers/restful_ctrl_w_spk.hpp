#pragma once

#include <crudpp/bindings/drogon/wrappers/restful_ctrl.hpp>

// single primary key and no authentication
template <typename T>
struct restful_ctrl<T, true, false> : public restful_ctrl_base<T>
{
    virtual void auth(const HttpRequestPtr& req,
                      std::function<void(const HttpResponsePtr &)>&& callback)
    {
        this->error(callback, k404NotFound);
    }

    void get_one(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr &)>&& callback,
                 typename model<T>::PrimaryKeyType&& id)
    {
        async_run(
            [
                callbackPtr =
                std::make_shared<std::function<void(const HttpResponsePtr &)>>(
                    std::move(callback)),
                id,
                this
        ] -> Task<>
            {
                auto dbClientPtr = this->getDbClient();
                drogon::orm::CoroMapper<model<T>> mapper(dbClientPtr);
                try
                {
                    auto r = co_await mapper.findByPrimaryKey(id);
                    (*callbackPtr)(HttpResponse::newHttpJsonResponse(r.toJson()));

                }
                catch (const DrogonDbException& e)
                {
                    const drogon::orm::UnexpectedRows *s=
                        dynamic_cast<const drogon::orm::UnexpectedRows *>(&e.base());

                    if (s)
                        this->error(callbackPtr, k404NotFound);
                    else
                        this->internal_error(e, callbackPtr);
                }

                co_return;
            });
    }

    void update_one(const HttpRequestPtr& req,
                   std::function<void(const HttpResponsePtr &)>&& callback,
                   typename model<T>::PrimaryKeyType&& id)
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

        async_run(
            [
                object,
                callbackPtr =
                std::make_shared<std::function<void(const HttpResponsePtr &)>>(
                    std::move(callback)),
                this
        ] -> Task<>
            {
                co_await this->db_update(object, callbackPtr);
                co_return;
            });
    }

    void delete_one(const HttpRequestPtr& req,
                   std::function<void(const HttpResponsePtr &)>&& callback,
                   typename model<T>::PrimaryKeyType&& id)
    {
        async_run(
            [
                callbackPtr =
                std::make_shared<std::function<void(const HttpResponsePtr &)>>(
                    std::move(callback)),
                id,
                this
        ] -> Task<>
            {
                co_await this->db_delete(id, callbackPtr);
                co_return;
            });
    }
};
