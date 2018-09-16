#include "stdafx.h"
#include "ProgressBarCollection.h"
#include "Exceptions.h"

namespace sf::lib
{
    ProgressBarCollection::ProgressBarCollection(size_t end)
        : m_end(end)
    {
    }

    bool ProgressBarCollection::AddCallback(std::string cbName, OnProgressCb cb)
    {
        return m_callbacks.emplace(std::move(cbName), std::move(cb)).second;
    }

    void ProgressBarCollection::RemoveCallback(std::string cbName)
    {
        m_callbacks.erase(std::move(cbName));
    }

    void ProgressBarCollection::Clear() noexcept
    {
        m_callbacks.clear();
    }

    void ProgressBarCollection::OnProgressChange(size_t current)
    {
        THROW_IF(current >= m_end, "Wrong value for progress change event [current=" << current
            << "] [end=" << m_end << "]");

        for (auto& cb : m_callbacks)
        {
            cb.second(current, m_end);
        }
    }
}