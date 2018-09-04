#pragma once
#include "INeedleCache.h"

namespace sf::lib
{
    class SortedNeedleCache : public INeedleCache
    {
    public:
        SortedNeedleCache(std::wstring needlePath);

        const OffsetList& GetOffsetList(char c) const override;
        const Data& GetNeedle() const override;

    private:
        Data m_needle;
        std::map<char, OffsetList> m_cache;
        mutable std::future<void> m_initWait;

        // return this index list if we did not find character in our cache
        OffsetList m_emptyIndexList;
    };
}

