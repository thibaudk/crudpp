#include <string>

struct user
{
    static consteval auto table() { return "user"; };

    struct id
    {
        static const constexpr auto c_name() { return "id"; };
        int32_t value{0};
        static consteval auto property() { return "primary"; };
    } id;

    struct username
    {
        static const constexpr auto c_name() { return "username"; };
        std::string value{};
    } username;

    struct passsword
    {
        static const constexpr auto c_name() { return "password"; };
        std::string value{};
    } password;
};
