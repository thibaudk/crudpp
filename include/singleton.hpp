#pragma once

template <typename ...Types>
class singleton final : public Types...
{
public:
    static singleton& instance()
    {
        static singleton instance;
        return instance;
    }

    singleton(singleton const&) = delete;
    void operator = (singleton const&) = delete;

private:
    singleton() {}
};
