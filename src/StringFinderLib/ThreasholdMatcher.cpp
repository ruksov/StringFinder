#include "stdafx.h"
#include "ThreasholdMatcher.h"
#include "Exceptions.h"

namespace sf::lib
{
    ThreasholdMatcher::ThreasholdMatcher(size_t threashold, CachePtr cache)
        : m_threashold(threashold)
        , m_cache(std::move(cache))
    {
        THROW_IF(!m_cache, "Failed to create threashold matcher. Cache object is null.");
        THROW_IF(m_threashold > m_cache->GetCacheData().size(), "Failed to create threashold matcher. Threashold can't be greater than needle file size.");
    }

    size_t ThreasholdMatcher::Match(size_t hsOffset, size_t hsDataIndex, const Data & hsData)
    {
        MatchResult maxRes(hsDataIndex);

        if (m_cachedMatchRes)
        {
            maxRes = GetMaxResult(hsOffset, m_cachedMatchRes.value(), hsData);
            m_cachedMatchRes.reset();
        }
        else
        {
            maxRes = GetMaxResult(hsOffset, hsDataIndex, hsData);
        }
        
        if (maxRes.HsDataOffset == hsOffset
            && maxRes.HsDataOffset + maxRes.MatchLen == hsData.size())
        {
            m_cachedMatchRes = maxRes;
            m_cachedMatchRes->HsDataIndex = hsDataIndex;
            return maxRes.MatchLen;
        }

        if (maxRes.MatchLen >= m_threashold)
        {
            NotifyAll(maxRes);

            if (maxRes.HsDataIndex != hsDataIndex)
            {
                // find result from previous chunk
                // so we need to fix match length
                return maxRes.HsDataOffset + maxRes.MatchLen - m_cache->GetCacheData().size();
            }
            else
            {
                return maxRes.MatchLen;
            }
        }

        if (maxRes.MatchLen > 2 && maxRes.HsDataIndex == hsDataIndex)
        {
            m_cachedMatchRes = maxRes;
        }

        return 0;
    }

    MatchResult ThreasholdMatcher::GetMaxResult(size_t hsOffset, size_t hsDataIndex, const Data & hsData)
    {
        MatchResult maxRes(hsDataIndex);
        maxRes.HsDataOffset = hsOffset;

        auto cacheRes = m_cache->GetFirstResult(hsOffset, hsData);
        if (!cacheRes)
        {
            return maxRes;
        }

        CacheMatchResult tmpRes = cacheRes.value();
        while (m_cache->GetNextResult(tmpRes, hsData));

        return maxRes = tmpRes;
    }

    MatchResult ThreasholdMatcher::GetMaxResult(size_t hsOffset, 
        const MatchResult & cachedMatchRes, 
        const Data & hsData)
    {
        MatchResult maxRes = cachedMatchRes;

        if (hsOffset == 0)
        {
            // inner cache search next match result based on previous data chunck,
            // so we subtract matchLen to create "virtual" representation of previous data chunck
            maxRes.HsDataOffset = 0 - maxRes.MatchLen;
        }
        else if (maxRes.HsDataOffset + 1 == hsOffset)
        {
            maxRes.HsDataOffset = hsOffset;
            ++maxRes.NlOffset;
            --maxRes.MatchLen;
        }
        else
        {
            THROW("Some error."
                << "Current hs offset - " << hsOffset << '\n'
                << "Cached match result:\n"
                << "hs index - " << maxRes.HsDataIndex << '\n'
                << "hs offset - " << maxRes.HsDataOffset << '\n'
                << "nl offset - " << maxRes.NlOffset << '\n'
                << "match len - " << maxRes.MatchLen);
        }
        
        auto cacheRes = maxRes.GetCacheMatchRes();
        while (m_cache->GetNextResult(cacheRes, hsData));
        
        maxRes = cacheRes;
        if (maxRes.HsDataOffset != hsOffset)
        {
            maxRes.HsDataOffset = cachedMatchRes.HsDataOffset;
        }
        return maxRes;
    }
}