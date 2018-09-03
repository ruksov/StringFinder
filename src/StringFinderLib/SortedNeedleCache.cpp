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
            size_t i = 0;
            for (auto& c : ndl)
            {
                const auto it = cache.emplace(c, 0);
                it.first->second.push_back(i);
                ++i;
            }
        });
    }

    const OffsetList & SortedNeedleCache::GetOffsetList(char c) const
    {
        if (m_initWait.valid())
        {
            m_initWait.get();
        }

        const auto it = m_cache.find(c);

        // if we find chracter it's offset list must be sorted
        assert(it == m_cache.end() 
            || (it != m_cache.end() && std::is_sorted(it->second.begin(), it->second.end())));

        return it != m_cache.end() ? it->second : m_emptyIndexList;
    }

    const Data & SortedNeedleCache::GetNeedle() const noexcept
    {
        return m_needle;
    }
}