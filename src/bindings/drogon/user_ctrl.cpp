#include <crudpp/bindings/drogon/wrappers/user_ctrl.hpp>
#include <crudpp/bindings/drogon/visitors/json_handler.hpp>

void user_ctrl::auth(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr &)>&& callback)
{
    auto jsonPtr=req->jsonObject();
    if(!jsonPtr)
    {
        this->error(callback,
                    "No json object is found in the request",
                    k400BadRequest);
        return;
    }

    // TODO: can be done better ?
    user tmp{};
    auto& uname{tmp.username};
    auto& pwd{tmp.password};
    bool dirtyFlag_[2] = { false };
    drgn::json_handler handler{dirtyFlag_, *jsonPtr};

    handler(uname);
    handler(pwd);

    if(!dirtyFlag_[0] || !dirtyFlag_[1])
    {
        this->error(callback,
                    "missing username and/or password in the request",
                    k400BadRequest);
        return;
    }

    async_run(
        [
            req,
            uname,
            pwd,
            callbackPtr =
            std::make_shared<std::function<void(const HttpResponsePtr &)>>(
                std::move(callback)),
            this
    ] mutable -> Task<>
        {
            auto dbClientPtr = this->getDbClient();
            drogon::orm::CoroMapper<model<user>> mapper(dbClientPtr);

            try
            {
                auto r = co_await mapper.findOne(
                    Criteria{std::remove_reference_t<decltype(uname)>::c_name(),
                             CompareOperator::EQ,
                             uname.value});

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
                    this->error(callbackPtr, "incorrect credentials", k401Unauthorized);
                    co_return;
                }

                r.get_aggregate().session_id.value = req->session()->sessionId();
                r.reset_flags();

                try
                {
                    co_await mapper.update(r);
                    (*callbackPtr)(HttpResponse::newHttpJsonResponse(this->makeJson(req, r)));
                }
                catch (const DrogonDbException &e)
                { this->internal_error(e, callbackPtr); }
            }
            catch (const DrogonDbException &e)
            {
                const drogon::orm::UnexpectedRows *s=
                    dynamic_cast<const drogon::orm::UnexpectedRows *>(&e.base());

                if (s)
                    this->error(callbackPtr, "incorrect credentials", k401Unauthorized);
                else
                    this->internal_error(e, callbackPtr);

                co_return;
            }
        });
}
