#pragma once
#include "ICache.h"

namespace sf::lib
{
    enum class CacheType
    {
        DiffCache = 0,
        Unknown
    };

    using CachePtr = std::unique_ptr<ICache>;

    CachePtr CacheFactory(CacheType cacheType, Data cacheData);
}