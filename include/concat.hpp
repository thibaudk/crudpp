#pragma once

// constexpr char* concatenation adapted from https://stackoverflow.com/a/53190554/14999126
// http://coliru.stacked-crooked.com/a/3f557060cc18719e

#include <concepts>

template <typename T>
concept has_left_right = requires()
{
    { T::left() } -> std::same_as<const char*>;
    { T::right() } -> std::same_as<const char*>;
};

template <size_t N>
struct String
{
    char c[N];
};

template <has_left_right T>
consteval auto concat()
{
    constexpr size_t length{std::char_traits<char>::length(T::left()) +
                            std::char_traits<char>::length(T::right())};

    String<length + 1> result{};
    result.c[length] = '\0';

    char* dst = result.c;

    const char* left{T::left()};
    for (; *left != '\0'; left++, dst++)
        *dst = *left;

    const char* right{T::right()};
    for (; *right != '\0'; right++, dst++)
        *dst = *right;

    return result;
}
