#include <string>

struct user
{
    static const constexpr auto table{"user"};

    struct id
    {
        static const constexpr auto name{"id"};
        int32_t value{0};
        bool updated{false};
        static const constexpr auto property{"primary"};
    } id;

    struct username
    {
        static const constexpr auto name{"username"};
        std::string value{};
        bool updated{false};
    } username;

    struct passsword
    {
        static const constexpr auto name{"password"};
        std::string value{};
        bool updated;
    } password;
};
