#pragma once

#include <crudpp/bindigs/drogon/wrappers/restful_ctrl_base.hpp>

// no single primary key and no authentication
template <typename T, bool has_single_primary_key = false, bool authenticating = false>
struct restful_ctrl : public restful_ctrl_base<T>
{
    virtual void auth(const HttpRequestPtr &req,
                      std::function<void(const HttpResponsePtr &)> &&callback)
    {
        this->error(callback, k404NotFound);
    }

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

            auto callbackPtr =
                std::make_shared<std::function<void(const HttpResponsePtr &)>>(
                    std::move(callback));

            this->db_update(object, callbackPtr);
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

            std::remove_const_t<typename t_trait<T>::pk_v_type> id{};

            bool parsed = [req, &id] <size_t... I> (index_sequence<I...>)
            {
                // get composite key from request parameters
                auto& parameters = req->parameters();
                auto id_names{t_trait<T>::pk_name()};

                return (asign_from_params<I>(get<I>(id), parameters, id_names) && ...);
            }
            (make_index_sequence<tuple_size_v<typename t_trait<T>::pk_v_type>>{});

            if (!parsed)
            {
                this->error(callback, k400BadRequest);
                return;
            }

            if (decltype(id){} == id)
            {
                this->error(callback, "composite key not set in parameters", k400BadRequest);
                return;
            }

            auto callbackPtr =
                std::make_shared<std::function<void(const HttpResponsePtr &)>>(
                    std::move(callback));

            this->db_delete(id, callbackPtr);
        }
        else
            this->error(callback, k404NotFound);
    }
};
