#include <cstdint>
#include <vector>

struct carts_products
{
    static consteval auto table() { return "carts_products"; };

    struct cart_id
    {
        static const constexpr auto c_name() { return "product_id"; };
        int32_t value{0};
    } cart_id;

    struct product_id
    {
        static const constexpr auto c_name() { return "product_id"; };
        int32_t value{0};
    } product_id;
};
