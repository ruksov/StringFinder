#pragma once
#include <optional>

#include "Result.h"

namespace sf::lib
{
    using Data = std::string;
    struct CacheMatchResult
    {
        size_t CacheOffset = 0;
        size_t CmpDataOffset = 0;
        size_t MatchLen = 0;

        CacheMatchResult(size_t cacheOffset, size_t cmpDataOffset, size_t matchLen)
            : CacheOffset(cacheOffset)
            , CmpDataOffset(cmpDataOffset)
            , MatchLen(matchLen)
        {
        }
    };

    struct ICache
    {
        virtual ~ICache() = default;
        
        virtual const Data& GetCacheData() const = 0;

        // reset inner cache
        virtual void Reset(Data cacheData) = 0;

        // return first possible match result with passed data
        virtual std::optional<CacheMatchResult> GetFirstResult(size_t cmpDataOffset, 
            const Data& cmpData) const = 0;

        // return next possible match result with passed data after previous result
        virtual std::optional<CacheMatchResult> GetNextResult(const CacheMatchResult& prevRes, 
            const Data& cmpData) const = 0;
    };
}