#include "stdafx.h"
#include "SortedNeedleCache.h"

namespace sf::lib
{
    SortedNeedleCache::SortedNeedleCache(std::string needle)
        : m_needle(std::move(needle))
    {
        m_initWait = std::async(std::launch::async, 
            [&ndl = m_needle, &cache = m_cache]()
        {
            uint32_t i = 0;
            for (auto& c : ndl)
            {
                auto it = cache.emplace(c, 0);
                it.first->second.push_back(i);
                ++i;
            }
        });
    }

    const IndexList & SortedNeedleCache::GetIndexList(char c) const
    {
        if (m_initWait.valid())
        {
            m_initWait.get();
        }

        auto it = m_cache.find(c);

        // if we find chracter it's index list must be sorted
        assert(it == m_cache.end() 
            || (it != m_cache.end() && std::is_sorted(it->second.begin(), it->second.end())));

        return it != m_cache.end() ? it->second : m_emptyIndexList;
    }

    const std::string & SortedNeedleCache::GetNeedle() const noexcept
    {
        return m_needle;
    }
}