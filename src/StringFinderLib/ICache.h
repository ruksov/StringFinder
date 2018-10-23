#pragma once
#include <optional>

#include "Result.h"

namespace sf::lib
{
    using Data = std::string;

    struct ICache
    {
        virtual ~ICache() = default;
        
        virtual const Data& GetCacheData() const = 0;

        // reset inner cache
        virtual void Reset(Data cacheData) = 0;

        // return first possible match result with passed data
        virtual std::optional<Result> GetFirstResult(size_t cmpDataOffset, const Data& cmpData) const = 0;

        // return next possible match result with passed data after previous result
        virtual std::optional<Result> GetNextResult(const Result& prevRes, const Data& cmpData) const = 0;
    };
}