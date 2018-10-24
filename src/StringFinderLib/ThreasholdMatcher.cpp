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

        if (maxRes.HsDataIndex == hsDataIndex 
            && maxRes.HsDataOffset + maxRes.MatchLen == hsData.size())
        {
            m_matchResFromPrevChunck = maxRes;
            matchLen = maxRes.MatchLen;
        }
        else if (maxRes.MatchLen >= m_threashold)
        {
            // notify all observers 
            // we find match result
            NotifyAll(maxRes);

            if (maxRes.HsDataIndex != hsDataIndex)
            {
                // find result from previous chunk
                // so we need to fix match length
                matchLen = maxRes.HsDataOffset + maxRes.MatchLen - m_cache->GetCacheData().size();
            }
            else
            {
                matchLen = maxRes.MatchLen;
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
        MatchResult maxRes = cachedMatchRes;

        // inner cache search next match result based on previous data chunck,
        // so we subtract matchLen to create "virtual" representation of previous data chunck
        maxRes.HsDataOffset = 0;
        maxRes.HsDataOffset -= maxRes.MatchLen;

        // try to match more bytes in current needle range
        auto& needleData = m_cache->GetCacheData();
        for (; maxRes.NlOffset + maxRes.MatchLen < needleData.size()
            && needleData.at(maxRes.NlOffset + maxRes.MatchLen) == hsData.at(maxRes.HsDataOffset + maxRes.MatchLen)
            ; ++maxRes.MatchLen);
        
        auto cacheRes = m_cache->GetNextResult(maxRes.GetCacheMatchRes(), hsData);
        while (cacheRes)
        {
            maxRes = cacheRes.value();
            cacheRes = m_cache->GetNextResult(cacheRes.value(), hsData);
        }

        maxRes.HsDataOffset = cachedMatchRes.HsDataOffset;
        return maxRes;
    }
}