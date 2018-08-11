#pragma once
#include "INeedleCache.h"

namespace sf::lib
{
    class SortedNeedleCache : public INeedleCache
    {
    public:
        SortedNeedleCache(std::string needle);

        const IndexList& GetIndexList(char c) const override;
        const std::string& GetNeedle() const noexcept override;

    private:
        std::string m_needle;
        std::map<char, IndexList> m_cache;
        mutable std::future<void> m_initWait;

        // return this index list if we did not find character in our cache
        IndexList m_emptyIndexList;
    };
}

