/**
 *
 *  CartsProducts.cc
 *  DO NOT EDIT. This file is generated by drogon_ctl
 *
 */

#include "CartsProducts.h"
#include <drogon/utils/Utilities.h>
#include <string>

using namespace drogon;
using namespace drogon::orm;
using namespace drogon_model::example;

const std::string CartsProducts::Cols::_cart_id = "cart_id";
const std::string CartsProducts::Cols::_product_id = "product_id";
const std::vector<std::string> CartsProducts::primaryKeyName = {"cart_id","product_id"};
const bool CartsProducts::hasPrimaryKey = true;
const std::string CartsProducts::tableName = "carts_products";

const std::vector<typename CartsProducts::MetaData> CartsProducts::metaData_={
{"cart_id","int32_t","int(11)",4,0,1,1},
{"product_id","int32_t","int(11)",4,0,1,1}
};
const std::string &CartsProducts::getColumnName(size_t index) noexcept(false)
{
    assert(index < metaData_.size());
    return metaData_[index].colName_;
}
CartsProducts::CartsProducts(const Row &r, const ssize_t indexOffset) noexcept
{
    if(indexOffset < 0)
    {
        if(!r["cart_id"].isNull())
        {
            cartId_=std::make_shared<int32_t>(r["cart_id"].as<int32_t>());
        }
        if(!r["product_id"].isNull())
        {
            productId_=std::make_shared<int32_t>(r["product_id"].as<int32_t>());
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
            cartId_=std::make_shared<int32_t>(r[index].as<int32_t>());
        }
        index = offset + 1;
        if(!r[index].isNull())
        {
            productId_=std::make_shared<int32_t>(r[index].as<int32_t>());
        }
    }

}

CartsProducts::CartsProducts(const Json::Value &pJson, const std::vector<std::string> &pMasqueradingVector) noexcept(false)
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
            cartId_=std::make_shared<int32_t>((int32_t)pJson[pMasqueradingVector[0]].asInt64());
        }
    }
    if(!pMasqueradingVector[1].empty() && pJson.isMember(pMasqueradingVector[1]))
    {
        dirtyFlag_[1] = true;
        if(!pJson[pMasqueradingVector[1]].isNull())
        {
            productId_=std::make_shared<int32_t>((int32_t)pJson[pMasqueradingVector[1]].asInt64());
        }
    }
}

CartsProducts::CartsProducts(const Json::Value &pJson) noexcept(false)
{
    if(pJson.isMember("cart_id"))
    {
        dirtyFlag_[0]=true;
        if(!pJson["cart_id"].isNull())
        {
            cartId_=std::make_shared<int32_t>((int32_t)pJson["cart_id"].asInt64());
        }
    }
    if(pJson.isMember("product_id"))
    {
        dirtyFlag_[1]=true;
        if(!pJson["product_id"].isNull())
        {
            productId_=std::make_shared<int32_t>((int32_t)pJson["product_id"].asInt64());
        }
    }
}

void CartsProducts::updateByMasqueradedJson(const Json::Value &pJson,
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
            cartId_=std::make_shared<int32_t>((int32_t)pJson[pMasqueradingVector[0]].asInt64());
        }
    }
    if(!pMasqueradingVector[1].empty() && pJson.isMember(pMasqueradingVector[1]))
    {
        if(!pJson[pMasqueradingVector[1]].isNull())
        {
            productId_=std::make_shared<int32_t>((int32_t)pJson[pMasqueradingVector[1]].asInt64());
        }
    }
}

void CartsProducts::updateByJson(const Json::Value &pJson) noexcept(false)
{
    if(pJson.isMember("cart_id"))
    {
        if(!pJson["cart_id"].isNull())
        {
            cartId_=std::make_shared<int32_t>((int32_t)pJson["cart_id"].asInt64());
        }
    }
    if(pJson.isMember("product_id"))
    {
        if(!pJson["product_id"].isNull())
        {
            productId_=std::make_shared<int32_t>((int32_t)pJson["product_id"].asInt64());
        }
    }
}

const int32_t &CartsProducts::getValueOfCartId() const noexcept
{
    const static int32_t defaultValue = int32_t();
    if(cartId_)
        return *cartId_;
    return defaultValue;
}
const std::shared_ptr<int32_t> &CartsProducts::getCartId() const noexcept
{
    return cartId_;
}
void CartsProducts::setCartId(const int32_t &pCartId) noexcept
{
    cartId_ = std::make_shared<int32_t>(pCartId);
    dirtyFlag_[0] = true;
}

const int32_t &CartsProducts::getValueOfProductId() const noexcept
{
    const static int32_t defaultValue = int32_t();
    if(productId_)
        return *productId_;
    return defaultValue;
}
const std::shared_ptr<int32_t> &CartsProducts::getProductId() const noexcept
{
    return productId_;
}
void CartsProducts::setProductId(const int32_t &pProductId) noexcept
{
    productId_ = std::make_shared<int32_t>(pProductId);
    dirtyFlag_[1] = true;
}

void CartsProducts::updateId(const uint64_t id)
{
}
typename CartsProducts::PrimaryKeyType CartsProducts::getPrimaryKey() const
{
    return std::make_tuple(*cartId_,*productId_);
}

const std::vector<std::string> &CartsProducts::insertColumns() noexcept
{
    static const std::vector<std::string> inCols={
        "cart_id",
        "product_id"
    };
    return inCols;
}

void CartsProducts::outputArgs(drogon::orm::internal::SqlBinder &binder) const
{
    if(dirtyFlag_[0])
    {
        if(getCartId())
        {
            binder << getValueOfCartId();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[1])
    {
        if(getProductId())
        {
            binder << getValueOfProductId();
        }
        else
        {
            binder << nullptr;
        }
    }
}

const std::vector<std::string> CartsProducts::updateColumns() const
{
    std::vector<std::string> ret;
    if(dirtyFlag_[0])
    {
        ret.push_back(getColumnName(0));
    }
    if(dirtyFlag_[1])
    {
        ret.push_back(getColumnName(1));
    }
    return ret;
}

void CartsProducts::updateArgs(drogon::orm::internal::SqlBinder &binder) const
{
    if(dirtyFlag_[0])
    {
        if(getCartId())
        {
            binder << getValueOfCartId();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[1])
    {
        if(getProductId())
        {
            binder << getValueOfProductId();
        }
        else
        {
            binder << nullptr;
        }
    }
}
Json::Value CartsProducts::toJson() const
{
    Json::Value ret;
    if(getCartId())
    {
        ret["cart_id"]=getValueOfCartId();
    }
    else
    {
        ret["cart_id"]=Json::Value();
    }
    if(getProductId())
    {
        ret["product_id"]=getValueOfProductId();
    }
    else
    {
        ret["product_id"]=Json::Value();
    }
    return ret;
}

Json::Value CartsProducts::toMasqueradedJson(
    const std::vector<std::string> &pMasqueradingVector) const
{
    Json::Value ret;
    if(pMasqueradingVector.size() == 2)
    {
        if(!pMasqueradingVector[0].empty())
        {
            if(getCartId())
            {
                ret[pMasqueradingVector[0]]=getValueOfCartId();
            }
            else
            {
                ret[pMasqueradingVector[0]]=Json::Value();
            }
        }
        if(!pMasqueradingVector[1].empty())
        {
            if(getProductId())
            {
                ret[pMasqueradingVector[1]]=getValueOfProductId();
            }
            else
            {
                ret[pMasqueradingVector[1]]=Json::Value();
            }
        }
        return ret;
    }
    LOG_ERROR << "Masquerade failed";
    if(getCartId())
    {
        ret["cart_id"]=getValueOfCartId();
    }
    else
    {
        ret["cart_id"]=Json::Value();
    }
    if(getProductId())
    {
        ret["product_id"]=getValueOfProductId();
    }
    else
    {
        ret["product_id"]=Json::Value();
    }
    return ret;
}

bool CartsProducts::validateJsonForCreation(const Json::Value &pJson, std::string &err)
{
    if(pJson.isMember("cart_id"))
    {
        if(!validJsonOfField(0, "cart_id", pJson["cart_id"], err, true))
            return false;
    }
    else
    {
        err="The cart_id column cannot be null";
        return false;
    }
    if(pJson.isMember("product_id"))
    {
        if(!validJsonOfField(1, "product_id", pJson["product_id"], err, true))
            return false;
    }
    else
    {
        err="The product_id column cannot be null";
        return false;
    }
    return true;
}
bool CartsProducts::validateMasqueradedJsonForCreation(const Json::Value &pJson,
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
        else
        {
            err="The " + pMasqueradingVector[0] + " column cannot be null";
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
        else
        {
            err="The " + pMasqueradingVector[1] + " column cannot be null";
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
bool CartsProducts::validateJsonForUpdate(const Json::Value &pJson, std::string &err)
{
    if(pJson.isMember("cart_id"))
    {
        if(!validJsonOfField(0, "cart_id", pJson["cart_id"], err, false))
            return false;
    }
    else
    {
        err = "The value of primary key must be set in the json object for update";
        return false;
    }
    if(pJson.isMember("product_id"))
    {
        if(!validJsonOfField(1, "product_id", pJson["product_id"], err, false))
            return false;
    }
    else
    {
        err = "The value of primary key must be set in the json object for update";
        return false;
    }
    return true;
}
bool CartsProducts::validateMasqueradedJsonForUpdate(const Json::Value &pJson,
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
    else
    {
        err = "The value of primary key must be set in the json object for update";
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
bool CartsProducts::validJsonOfField(size_t index,
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
            if(!pJson.isInt())
            {
                err="Type error in the "+fieldName+" field";
                return false;
            }
            break;
        case 1:
            if(pJson.isNull())
            {
                err="The " + fieldName + " column cannot be null";
                return false;
            }
            if(!pJson.isInt())
            {
                err="Type error in the "+fieldName+" field";
                return false;
            }
            break;
        default:
            err="Internal error in the server";
            return false;
    }
    return true;
}