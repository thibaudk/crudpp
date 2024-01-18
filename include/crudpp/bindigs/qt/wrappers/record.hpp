#pragma once

namespace qt
{
template <typename T, bool has_primary_key = false>
struct record
{
protected:
    record() = default;

    // false by default to siginfy it has not been inserted yet
    bool m_inserted{false};
}
