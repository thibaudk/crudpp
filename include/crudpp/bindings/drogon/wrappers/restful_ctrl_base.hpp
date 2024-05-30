#pragma once

#include <drogon/HttpController.h>
#include <drogon/orm/RestfulController.h>

#include <crudpp/concepts.hpp>
#include <crudpp/bindings/drogon/wrappers/model.hpp>

using namespace drogon;
using namespace drgn;

template <typename T>
class restful_ctrl_base : public RestfulController
{
    using callback_ptr =
        const std::shared_ptr<std::function<void (const std::shared_ptr<drogon::HttpResponse> &)>>;

    orm::Criteria make_criteria(const SafeStringMap<std::string>& parameters, int found_params = 0)
    {
        orm::Criteria crit;

        auto iter = parameters.find("*");
        if (iter != parameters.end())
        {
            found_params++;

            boost::pfr::for_each_field(T{},
                                       [&parameters, &crit, p = '%' + iter->second + '%']
                                       (const crudpp::r_c_n_v auto & f)
                                       {
                                           using namespace std;

                                           if constexpr (same_as<decltype(f.value), string>)
                                               crit = crit ||
                                                      Criteria(f.c_name(), CompareOperator::Like, p);
                                       }
                                       );
        }

        return crit;
    }

public:
    void get(const HttpRequestPtr &req,
             std::function<void(const HttpResponsePtr &)> &&callback)
    {
        auto dbClientPtr = getDbClient();
        Mapper<model<T>> mapper(dbClientPtr);
        auto &parameters = req->parameters();
        int found_params{0};

        auto iter = parameters.find("sort");
        if(iter != parameters.end())
        {
            found_params++;

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
            found_params++;

            try{
                auto offset = std::stoll(iter->second);
                mapper.offset(offset);
            }
            catch(...)
            {
                error(callback, k400BadRequest);
                return;
            }
        }
        iter = parameters.find("limit");
        if(iter != parameters.end())
        {
            found_params++;

            try{
                auto limit = std::stoll(iter->second);
                mapper.limit(limit);
            }
            catch(...)
            {
                error(callback, k400BadRequest);
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
                auto criteria = makeCriteria((*jsonPtr)["filter"])
                                && make_criteria(parameters, found_params);

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
                    [callbackPtr, this](const DrogonDbException &e) { internal_error(e, callbackPtr); });
            }
            catch(const std::exception &e)
            {
                LOG_ERROR << e.what();
                error(callback, e.what(), k400BadRequest);
                return;
            }
        }
        else
        {
            if (parameters.size() == found_params)
            {
                mapper.findAll(
                    [req, callbackPtr, this](const std::vector<model<T>> &v)
                    {
                        Json::Value ret;
                        ret.resize(0);
                        for (auto &obj : v)
                        {
                            ret.append(makeJson(req, obj));
                        }
                        (*callbackPtr)(HttpResponse::newHttpJsonResponse(ret));
                    },
                    [callbackPtr, this](const DrogonDbException &e) { internal_error(e, callbackPtr); });
            }
            else
            {
                auto criteria{make_criteria(parameters, found_params)};

                mapper.findBy(criteria,
                    [req, callbackPtr, this](const std::vector<model<T>> &v)
                    {
                        Json::Value ret;
                        ret.resize(0);
                        for (auto &obj : v)
                        {
                            ret.append(makeJson(req, obj));
                        }
                        (*callbackPtr)(HttpResponse::newHttpJsonResponse(ret));
                    },
                    [callbackPtr, this](const DrogonDbException &e) { internal_error(e, callbackPtr); });
            }
        }
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
                error(callback, err, k400BadRequest);
                return;
            }
        }

        try
        {
            model<T> object = model<T>(*jsonPtr);
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
                [callbackPtr, this](const DrogonDbException &e){ internal_error(e, callbackPtr); });
        }
        catch(const Json::Exception &e)
        {
            LOG_ERROR << e.what();
            error(callback, "Field type error", k400BadRequest);
            return;
        }
    }

    virtual void update_by(const HttpRequestPtr &req,
                  std::function<void(const HttpResponsePtr &)> &&callback)
    {
        error(callback, k404NotFound);
    }

    virtual void delete_by(const HttpRequestPtr &req,
                  std::function<void(const HttpResponsePtr &)> &&callback)
    {
        error(callback, k404NotFound);
    }

    /// Ensure that subclasses inherited from this class are instantiated.
    restful_ctrl_base() : RestfulController{ model<T>::insertColumns() }
    {
        disableMasquerading();
    }

protected:
    // const std::string dbClientName_{"default"};

    orm::DbClientPtr getDbClient()
    {
        // return drogon::app().getDbClient(dbClientName_);
        return drogon::app().getDbClient();
    }

    bool read_json(const std::function<void(const HttpResponsePtr &)>& callback,
                   std::shared_ptr<Json::Value> jsonPtr,
                   model<T>& object)
    {
        std::string err;
        if(!this->doCustomValidations(*jsonPtr, err))
        {
            error(callback, err, k400BadRequest);
            return false;
        }
        try
        {
            object.updateByJson(*jsonPtr);
        }
        catch (const Json::Exception &e)
        {
            LOG_ERROR << e.what();
            error(callback, "Field type error", k400BadRequest);
            return false;
        }

        return true;
    }

    void db_update(const model<T>& object, callback_ptr callbackPtr)
    {
        auto dbClientPtr = this->getDbClient();

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
                    error(callbackPtr, "No resources are updated", k404NotFound);
                else
                {
                    LOG_FATAL << "More than one resource is updated: " << count;
                    internal_error(callbackPtr);
                }
            },
            [callbackPtr, this](const DrogonDbException &e) { internal_error(e, callbackPtr); });
    }

    void db_delete(const typename model<T>::PrimaryKeyType& id, callback_ptr callbackPtr)
    {
        auto dbClientPtr = this->getDbClient();

        drogon::orm::Mapper<model<T>> mapper(dbClientPtr);
        mapper.deleteByPrimaryKey(
            id,
            [callbackPtr, this](const size_t count) {
                if(count == 1)
                    error(callbackPtr, k204NoContent);
                else if(count == 0)
                    error(callbackPtr, "No resources deleted", k404NotFound);
                else
                {
                    LOG_FATAL << "Delete more than one records: " << count;
                    internal_error(callbackPtr);
                }
            },
            [callbackPtr, this](const DrogonDbException &e) { internal_error(e, callbackPtr); });
    }

    void internal_error(const DrogonDbException &e, callback_ptr callbackPtr) const
    {
        LOG_ERROR << e.base().what();
        Json::Value ret;
        ret["error"] = "database error";
        auto resp = HttpResponse::newHttpJsonResponse(ret);
        resp->setStatusCode(k500InternalServerError);
        (*callbackPtr)(resp);
    }

    void internal_error(callback_ptr callbackPtr) const
    {
        Json::Value ret;
        ret["error"] = "database error";
        auto resp = HttpResponse::newHttpJsonResponse(ret);
        resp->setStatusCode(k500InternalServerError);
        (*callbackPtr)(resp);
    }

    void error(callback_ptr callbackPtr, HttpStatusCode status) const
    {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(status);
        (*callbackPtr)(resp);
    }

    void error(const std::function<void(const HttpResponsePtr &)>& callback,
               HttpStatusCode status) const
    {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(status);
        callback(resp);
    }

    void error(callback_ptr callbackPtr,
               const std::string& error_str,
               HttpStatusCode status = k500InternalServerError) const
    {
        Json::Value ret;
        ret["error"] = error_str;
        auto resp = HttpResponse::newHttpJsonResponse(ret);
        resp->setStatusCode(status);
        (*callbackPtr)(resp);
    }

    void error(callback_ptr callbackPtr,
               const char* error_str,
               HttpStatusCode status = k500InternalServerError) const
    {
        Json::Value ret;
        ret["error"] = error_str;
        auto resp = HttpResponse::newHttpJsonResponse(ret);
        resp->setStatusCode(status);
        (*callbackPtr)(resp);
    }

    void error(const std::function<void(const HttpResponsePtr &)>& callback,
               const char* error_str,
               HttpStatusCode status = k500InternalServerError) const
    {
        Json::Value ret;
        ret["error"] = error_str;
        auto resp = HttpResponse::newHttpJsonResponse(ret);
        resp->setStatusCode(status);
        callback(resp);
    }

    void error(const std::function<void(const HttpResponsePtr &)>& callback,
               const std::string& error_str,
               HttpStatusCode status = k500InternalServerError) const
    {
        Json::Value ret;
        ret["error"] = error_str;
        auto resp = HttpResponse::newHttpJsonResponse(ret);
        resp->setStatusCode(status);
        callback(resp);
    }
};
