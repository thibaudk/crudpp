/**
 *
 *  Products.cc
 *  DO NOT EDIT. This file is generated by drogon_ctl
 *
 */

#include "Products.h"
#include "Carts.h"
#include "CartsProducts.h"
#include "Reviews.h"
#include "Skus.h"
#include <drogon/utils/Utilities.h>
#include <string>

using namespace drogon;
using namespace drogon::orm;
using namespace drogon_model::example;

const std::string Products::Cols::_id = "id";
const std::string Products::Cols::_product_name = "product_name";
const std::string Products::primaryKeyName = "id";
const bool Products::hasPrimaryKey = true;
const std::string Products::tableName = "products";

const std::vector<typename Products::MetaData> Products::metaData_={
{"id","int32_t","int(11)",4,1,1,1},
{"product_name","std::string","varchar(63)",63,0,0,0}
};
const std::string &Products::getColumnName(size_t index) noexcept(false)
{
    assert(index < metaData_.size());
    return metaData_[index].colName_;
}
Products::Products(const Row &r, const ssize_t indexOffset) noexcept
{
    if(indexOffset < 0)
    {
        if(!r["id"].isNull())
        {
            id_=std::make_shared<int32_t>(r["id"].as<int32_t>());
        }
        if(!r["product_name"].isNull())
        {
            productName_=std::make_shared<std::string>(r["product_name"].as<std::string>());
        }
    }
    else
    {
        size_t offset = (size_t)indexOffset;
        if(offset + 2 > r.size())
        {
            LOG_FATAL << "Invalid SQL result for this model";
            return;
        }
        size_t index;
        index = offset + 0;
        if(!r[index].isNull())
        {
            id_=std::make_shared<int32_t>(r[index].as<int32_t>());
        }
        index = offset + 1;
        if(!r[index].isNull())
        {
            productName_=std::make_shared<std::string>(r[index].as<std::string>());
        }
    }

}

Products::Products(const Json::Value &pJson, const std::vector<std::string> &pMasqueradingVector) noexcept(false)
{
    if(pMasqueradingVector.size() != 2)
    {
        LOG_ERROR << "Bad masquerading vector";
        return;
    }
    if(!pMasqueradingVector[0].empty() && pJson.isMember(pMasqueradingVector[0]))
    {
        dirtyFlag_[0] = true;
        if(!pJson[pMasqueradingVector[0]].isNull())
        {
            id_=std::make_shared<int32_t>((int32_t)pJson[pMasqueradingVector[0]].asInt64());
        }
    }
    if(!pMasqueradingVector[1].empty() && pJson.isMember(pMasqueradingVector[1]))
    {
        dirtyFlag_[1] = true;
        if(!pJson[pMasqueradingVector[1]].isNull())
        {
            productName_=std::make_shared<std::string>(pJson[pMasqueradingVector[1]].asString());
        }
    }
}

Products::Products(const Json::Value &pJson) noexcept(false)
{
    if(pJson.isMember("id"))
    {
        dirtyFlag_[0]=true;
        if(!pJson["id"].isNull())
        {
            id_=std::make_shared<int32_t>((int32_t)pJson["id"].asInt64());
        }
    }
    if(pJson.isMember("product_name"))
    {
        dirtyFlag_[1]=true;
        if(!pJson["product_name"].isNull())
        {
            productName_=std::make_shared<std::string>(pJson["product_name"].asString());
        }
    }
}

void Products::updateByMasqueradedJson(const Json::Value &pJson,
                                            const std::vector<std::string> &pMasqueradingVector) noexcept(false)
{
    if(pMasqueradingVector.size() != 2)
    {
        LOG_ERROR << "Bad masquerading vector";
        return;
    }
    if(!pMasqueradingVector[0].empty() && pJson.isMember(pMasqueradingVector[0]))
    {
        if(!pJson[pMasqueradingVector[0]].isNull())
        {
            id_=std::make_shared<int32_t>((int32_t)pJson[pMasqueradingVector[0]].asInt64());
        }
    }
    if(!pMasqueradingVector[1].empty() && pJson.isMember(pMasqueradingVector[1]))
    {
        dirtyFlag_[1] = true;
        if(!pJson[pMasqueradingVector[1]].isNull())
        {
            productName_=std::make_shared<std::string>(pJson[pMasqueradingVector[1]].asString());
        }
    }
}

void Products::updateByJson(const Json::Value &pJson) noexcept(false)
{
    if(pJson.isMember("id"))
    {
        if(!pJson["id"].isNull())
        {
            id_=std::make_shared<int32_t>((int32_t)pJson["id"].asInt64());
        }
    }
    if(pJson.isMember("product_name"))
    {
        dirtyFlag_[1] = true;
        if(!pJson["product_name"].isNull())
        {
            productName_=std::make_shared<std::string>(pJson["product_name"].asString());
        }
    }
}

const int32_t &Products::getValueOfId() const noexcept
{
    static const int32_t defaultValue = int32_t();
    if(id_)
        return *id_;
    return defaultValue;
}
const std::shared_ptr<int32_t> &Products::getId() const noexcept
{
    return id_;
}
void Products::setId(const int32_t &pId) noexcept
{
    id_ = std::make_shared<int32_t>(pId);
    dirtyFlag_[0] = true;
}
const typename Products::PrimaryKeyType & Products::getPrimaryKey() const
{
    assert(id_);
    return *id_;
}

const std::string &Products::getValueOfProductName() const noexcept
{
    static const std::string defaultValue = std::string();
    if(productName_)
        return *productName_;
    return defaultValue;
}
const std::shared_ptr<std::string> &Products::getProductName() const noexcept
{
    return productName_;
}
void Products::setProductName(const std::string &pProductName) noexcept
{
    productName_ = std::make_shared<std::string>(pProductName);
    dirtyFlag_[1] = true;
}
void Products::setProductName(std::string &&pProductName) noexcept
{
    productName_ = std::make_shared<std::string>(std::move(pProductName));
    dirtyFlag_[1] = true;
}
void Products::setProductNameToNull() noexcept
{
    productName_.reset();
    dirtyFlag_[1] = true;
}

void Products::updateId(const uint64_t id)
{
    id_ = std::make_shared<int32_t>(static_cast<int32_t>(id));
}

const std::vector<std::string> &Products::insertColumns() noexcept
{
    static const std::vector<std::string> inCols={
        "product_name"
    };
    return inCols;
}

void Products::outputArgs(drogon::orm::internal::SqlBinder &binder) const
{
    if(dirtyFlag_[1])
    {
        if(getProductName())
        {
            binder << getValueOfProductName();
        }
        else
        {
            binder << nullptr;
        }
    }
}

const std::vector<std::string> Products::updateColumns() const
{
    std::vector<std::string> ret;
    if(dirtyFlag_[1])
    {
        ret.push_back(getColumnName(1));
    }
    return ret;
}

void Products::updateArgs(drogon::orm::internal::SqlBinder &binder) const
{
    if(dirtyFlag_[1])
    {
        if(getProductName())
        {
            binder << getValueOfProductName();
        }
        else
        {
            binder << nullptr;
        }
    }
}
Json::Value Products::toJson() const
{
    Json::Value ret;
    if(getId())
    {
        ret["id"]=getValueOfId();
    }
    else
    {
        ret["id"]=Json::Value();
    }
    if(getProductName())
    {
        ret["product_name"]=getValueOfProductName();
    }
    else
    {
        ret["product_name"]=Json::Value();
    }
    return ret;
}

Json::Value Products::toMasqueradedJson(
    const std::vector<std::string> &pMasqueradingVector) const
{
    Json::Value ret;
    if(pMasqueradingVector.size() == 2)
    {
        if(!pMasqueradingVector[0].empty())
        {
            if(getId())
            {
                ret[pMasqueradingVector[0]]=getValueOfId();
            }
            else
            {
                ret[pMasqueradingVector[0]]=Json::Value();
            }
        }
        if(!pMasqueradingVector[1].empty())
        {
            if(getProductName())
            {
                ret[pMasqueradingVector[1]]=getValueOfProductName();
            }
            else
            {
                ret[pMasqueradingVector[1]]=Json::Value();
            }
        }
        return ret;
    }
    LOG_ERROR << "Masquerade failed";
    if(getId())
    {
        ret["id"]=getValueOfId();
    }
    else
    {
        ret["id"]=Json::Value();
    }
    if(getProductName())
    {
        ret["product_name"]=getValueOfProductName();
    }
    else
    {
        ret["product_name"]=Json::Value();
    }
    return ret;
}

bool Products::validateJsonForCreation(const Json::Value &pJson, std::string &err)
{
    if(pJson.isMember("id"))
    {
        if(!validJsonOfField(0, "id", pJson["id"], err, true))
            return false;
    }
    if(pJson.isMember("product_name"))
    {
        if(!validJsonOfField(1, "product_name", pJson["product_name"], err, true))
            return false;
    }
    return true;
}
bool Products::validateMasqueradedJsonForCreation(const Json::Value &pJson,
                                                  const std::vector<std::string> &pMasqueradingVector,
                                                  std::string &err)
{
    if(pMasqueradingVector.size() != 2)
    {
        err = "Bad masquerading vector";
        return false;
    }
    try {
      if(!pMasqueradingVector[0].empty())
      {
          if(pJson.isMember(pMasqueradingVector[0]))
          {
              if(!validJsonOfField(0, pMasqueradingVector[0], pJson[pMasqueradingVector[0]], err, true))
                  return false;
          }
      }
      if(!pMasqueradingVector[1].empty())
      {
          if(pJson.isMember(pMasqueradingVector[1]))
          {
              if(!validJsonOfField(1, pMasqueradingVector[1], pJson[pMasqueradingVector[1]], err, true))
                  return false;
          }
      }
    }
    catch(const Json::LogicError &e)
    {
      err = e.what();
      return false;
    }
    return true;
}
bool Products::validateJsonForUpdate(const Json::Value &pJson, std::string &err)
{
    if(pJson.isMember("id"))
    {
        if(!validJsonOfField(0, "id", pJson["id"], err, false))
            return false;
    }
    else
    {
        err = "The value of primary key must be set in the json object for update";
        return false;
    }
    if(pJson.isMember("product_name"))
    {
        if(!validJsonOfField(1, "product_name", pJson["product_name"], err, false))
            return false;
    }
    return true;
}
bool Products::validateMasqueradedJsonForUpdate(const Json::Value &pJson,
                                                const std::vector<std::string> &pMasqueradingVector,
                                                std::string &err)
{
    if(pMasqueradingVector.size() != 2)
    {
        err = "Bad masquerading vector";
        return false;
    }
    try {
      if(!pMasqueradingVector[0].empty() && pJson.isMember(pMasqueradingVector[0]))
      {
          if(!validJsonOfField(0, pMasqueradingVector[0], pJson[pMasqueradingVector[0]], err, false))
              return false;
      }
    else
    {
        err = "The value of primary key must be set in the json object for update";
        return false;
    }
      if(!pMasqueradingVector[1].empty() && pJson.isMember(pMasqueradingVector[1]))
      {
          if(!validJsonOfField(1, pMasqueradingVector[1], pJson[pMasqueradingVector[1]], err, false))
              return false;
      }
    }
    catch(const Json::LogicError &e)
    {
      err = e.what();
      return false;
    }
    return true;
}
bool Products::validJsonOfField(size_t index,
                                const std::string &fieldName,
                                const Json::Value &pJson,
                                std::string &err,
                                bool isForCreation)
{
    switch(index)
    {
        case 0:
            if(pJson.isNull())
            {
                err="The " + fieldName + " column cannot be null";
                return false;
            }
            if(isForCreation)
            {
                err="The automatic primary key cannot be set";
                return false;
            }
            if(!pJson.isInt())
            {
                err="Type error in the "+fieldName+" field";
                return false;
            }
            break;
        case 1:
            if(pJson.isNull())
            {
                return true;
            }
            if(!pJson.isString())
            {
                err="Type error in the "+fieldName+" field";
                return false;
            }
            // asString().length() creates a string object, is there any better way to validate the length?
            if(pJson.isString() && pJson.asString().length() > 63)
            {
                err="String length exceeds limit for the " +
                    fieldName +
                    " field (the maximum value is 63)";
                return false;
            }

            break;
        default:
            err="Internal error in the server";
            return false;
    }
    return true;
}
Skus Products::getSKU(const DbClientPtr &clientPtr) const {
    static const std::string sql = "select * from skus where product_id = ?";
    Result r(nullptr);
    {
        auto binder = *clientPtr << sql;
        binder << *id_ << Mode::Blocking >>
            [&r](const Result &result) { r = result; };
        binder.exec();
    }
    if (r.size() == 0)
    {
        throw UnexpectedRows("0 rows found");
    }
    else if (r.size() > 1)
    {
        throw UnexpectedRows("Found more than one row");
    }
    return Skus(r[0]);
}

void Products::getSKU(const DbClientPtr &clientPtr,
                      const std::function<void(Skus)> &rcb,
                      const ExceptionCallback &ecb) const
{
    static const std::string sql = "select * from skus where product_id = ?";
    *clientPtr << sql
               << *id_
               >> [rcb = std::move(rcb), ecb](const Result &r){
                    if (r.size() == 0)
                    {
                        ecb(UnexpectedRows("0 rows found"));
                    }
                    else if (r.size() > 1)
                    {
                        ecb(UnexpectedRows("Found more than one row"));
                    }
                    else
                    {
                        rcb(Skus(r[0]));
                    }
               }
               >> ecb;
}
std::vector<Reviews> Products::getReviews(const DbClientPtr &clientPtr) const {
    static const std::string sql = "select * from reviews where product_id = ?";
    Result r(nullptr);
    {
        auto binder = *clientPtr << sql;
        binder << *id_ << Mode::Blocking >>
            [&r](const Result &result) { r = result; };
        binder.exec();
    }
    std::vector<Reviews> ret;
    ret.reserve(r.size());
    for (auto const &row : r)
    {
        ret.emplace_back(Reviews(row));
    }
    return ret;
}

void Products::getReviews(const DbClientPtr &clientPtr,
                          const std::function<void(std::vector<Reviews>)> &rcb,
                          const ExceptionCallback &ecb) const
{
    static const std::string sql = "select * from reviews where product_id = ?";
    *clientPtr << sql
               << *id_
               >> [rcb = std::move(rcb)](const Result &r){
                   std::vector<Reviews> ret;
                   ret.reserve(r.size());
                   for (auto const &row : r)
                   {
                       ret.emplace_back(Reviews(row));
                   }
                   rcb(ret);
               }
               >> ecb;
}
std::vector<std::pair<Carts,CartsProducts>> Products::getCarts(const DbClientPtr &clientPtr) const {
    static const std::string sql = "select * from carts,carts_products where carts_products.product_id = ? and carts_products.cart_id = carts.id";
    Result r(nullptr);
    {
        auto binder = *clientPtr << sql;
        binder << *id_ << Mode::Blocking >>
            [&r](const Result &result) { r = result; };
        binder.exec();
    }
    std::vector<std::pair<Carts,CartsProducts>> ret;
    ret.reserve(r.size());
    for (auto const &row : r)
    {
        ret.emplace_back(std::pair<Carts,CartsProducts>(
            Carts(row),CartsProducts(row,Carts::getColumnNumber())));
    }
    return ret;
}

void Products::getCarts(const DbClientPtr &clientPtr,
                        const std::function<void(std::vector<std::pair<Carts,CartsProducts>>)> &rcb,
                        const ExceptionCallback &ecb) const
{
    static const std::string sql = "select * from carts,carts_products where carts_products.product_id = ? and carts_products.cart_id = carts.id";
    *clientPtr << sql
               << *id_
               >> [rcb = std::move(rcb)](const Result &r){
                   std::vector<std::pair<Carts,CartsProducts>> ret;
                   ret.reserve(r.size());
                   for (auto const &row : r)
                   {
                       ret.emplace_back(std::pair<Carts,CartsProducts>(
                           Carts(row),CartsProducts(row,Carts::getColumnNumber())));
                   }
                   rcb(ret);
               }
               >> ecb;
}
