#include "stdafx.h"
#include "ThreasholdMatcher.h"

namespace sf::lib
{
    ThreasholdMatcher::ThreasholdMatcher(size_t threashold, CachePtr cache)
        : m_threashold(threashold)
        , m_cache(std::move(cache))
    {
    }

    size_t ThreasholdMatcher::Match(size_t hsOffset, size_t hsDataIndex, const Data & hsData)
    {
        MatchResult maxRes(hsDataIndex);

        if (hsOffset == 0 && m_matchResFromPrevChunck)
        {
            // try to end match action from previous haystack data chunck
            maxRes = GetMaxResult(m_matchResFromPrevChunck.value(), hsData);

            m_matchResFromPrevChunck.reset();
        }
        else
        {
            maxRes = GetMaxResult(hsOffset, hsDataIndex, hsData);
        }

        size_t matchLen = 0;
        if (maxRes.MatchLen > m_threashold)
        {
            matchLen = maxRes.MatchLen;

            if (maxRes.HsDataOffset + maxRes.MatchLen < hsData.size())
            {
                // notify all observers 
                // we find match result
                NotifyAll(maxRes);
            }
            else
            {
                m_matchResFromPrevChunck = maxRes;
            }
        }

        return matchLen;
    }

    MatchResult ThreasholdMatcher::GetMaxResult(size_t hsOffset, size_t hsDataIndex, const Data & hsData)
    {
        MatchResult maxRes(hsDataIndex);

        auto cacheRes = m_cache->GetFirstResult(hsOffset, hsData);
        while (cacheRes)
        {
            maxRes = cacheRes.value();
            cacheRes = m_cache->GetNextResult(cacheRes.value(), hsData);
        }

        return maxRes;
    }

    MatchResult ThreasholdMatcher::GetMaxResult(const MatchResult & cachedMatchRes, const Data & hsData)
    {
        MatchResult maxRes(cachedMatchRes.HsDataIndex);

        auto cacheRes = m_cache->GetNextResult(cachedMatchRes.GetCacheMatchRes(), hsData);
        while (cacheRes)
        {
            maxRes = cacheRes.value();
            cacheRes = m_cache->GetNextResult(cacheRes.value(), hsData);
        }

        return maxRes;
    }
}