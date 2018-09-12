#pragma once
#include <optional>
#include <memory>

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

        bool operator==(const CacheMatchResult& other) const
        {
            return CacheOffset == other.CacheOffset
                && CmpDataOffset == other.CmpDataOffset
                && MatchLen == other.MatchLen;
        }

        bool operator!=(const CacheMatchResult& other) const
        {
            return !operator==(other);
        }
    };

    struct ICache
    {
        virtual ~ICache() = default;
        
        virtual const Data& GetCacheData() const = 0;

        // reset inner cache
        virtual void Reset(Data cacheData) = 0;

		// If inOutRes specified it will be updated to more suitable result,
		// else it will be filled first match result in diff tree.
		// This result will be used to try find next result in GetNextResult.
		// Method returns true if inOutRes changed and can be used.
		virtual bool GetFirstResult(CacheMatchResult& inOutRes, const Data& cmpData) = 0;

        // inOutRes - next possible match result with passed data after previous result
        // Method returns true if inOutRes changed and can be used.
        virtual  bool GetNextResult(CacheMatchResult& inOutRes, const Data& cmpData) = 0;
    };

    using CachePtr = std::unique_ptr<ICache>;
}