#pragma once

#include <crudpp/bindings/drogon/wrappers/restful_ctrl_base.hpp>

// no single primary key
template <typename T, bool has_single_primary_key = false>
struct restful_ctrl : public restful_ctrl_base<T>
{
    void get_one(const HttpRequestPtr &req,
                std::function<void(const HttpResponsePtr &)> &&callback)
    {
        this->error(callback, k404NotFound);
    }

    void update_one(const HttpRequestPtr &req,
                   std::function<void(const HttpResponsePtr &)> &&callback)
    {
        this->error(callback, k404NotFound);
    }

    void update_by(const HttpRequestPtr &req,
                   std::function<void(const HttpResponsePtr &)> &&callback) override
    {
        if constexpr (crudpp::r_composite_primary_key<T>)
        {
            auto jsonPtr=req->jsonObject();
            if(!jsonPtr)
            {
                this->error(callback, "No json object is found in the request", k400BadRequest);
                return;
            }

            model<T> object;
            this->read_json(callback, jsonPtr, object);

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
        else
            this->error(callback, k404NotFound);
    }

    void delete_one(const HttpRequestPtr &req,
                   std::function<void(const HttpResponsePtr &)> &&callback)
    {
        this->error(callback, k404NotFound);
    }

    void delete_by(const HttpRequestPtr &req,
                   std::function<void(const HttpResponsePtr &)> &&callback) override
    {
        using namespace crudpp;

        if constexpr (r_composite_primary_key<T>)
        {
            using namespace std;

            std::remove_const_t<typename model<T>::PrimaryKeyType> id{};

            bool parsed = [req, &id] <size_t... I> (index_sequence<I...>)
            {
                // get composite key from request parameters
                auto& parameters = req->parameters();
                const auto id_names{model<T>::primaryKeyName};

                return (asign_from_params<I>(get<I>(id), parameters, id_names) && ...);
            }
            (make_index_sequence<pk_size<T>()>{});

            if (!parsed)
            {
                this->error(callback, k400BadRequest);
                return;
            }

            // FIXME: bad way to check for
            if (decltype(id){} == id)
            {
                this->error(callback, "composite key not set in parameters", k400BadRequest);
                return;
            }

            async_run(
                [
                    id,
                    callbackPtr =
                    std::make_shared<std::function<void(const HttpResponsePtr &)>>(
                        std::move(callback)),
                    this
            ] -> Task<>
                {
                    co_await this->db_delete(id, callbackPtr);
                    co_return;
                });
        }
        else
            this->error(callback, k404NotFound);
    }
};
