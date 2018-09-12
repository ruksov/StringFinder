#pragma once
#include "ICache.h"

namespace sf::lib
{
    struct MatchResult
    {
        // index of haystack data chunck
        size_t HsDataIndex = 0;

        // match information for current haystack data chunck
        size_t NlOffset = 0;
        size_t HsDataOffset = 0;
        size_t MatchLen = 0;

        explicit MatchResult(size_t hsDataIndex)
            : HsDataIndex(hsDataIndex)
        {
        }

        explicit MatchResult(size_t hsDataIndex, const CacheMatchResult& cacheMatchRes)
            : NlOffset(cacheMatchRes.CacheOffset)
            , HsDataOffset(cacheMatchRes.CmpDataOffset)
            , MatchLen(cacheMatchRes.MatchLen)
            , HsDataIndex(hsDataIndex)
        {
        }

        MatchResult& operator=(const CacheMatchResult& cacheMatchRes)
        {
            NlOffset = cacheMatchRes.CacheOffset;
            HsDataOffset = cacheMatchRes.CmpDataOffset;
            MatchLen = cacheMatchRes.MatchLen;

            return *this;
        }

        CacheMatchResult GetCacheMatchRes() const
        {
            return CacheMatchResult(NlOffset, HsDataOffset, MatchLen);
        }
    };
}