#pragma once
#include "ICache.h"

namespace sf::lib
{
    enum class CacheType
    {
        DiffCache = 0,
        Unknown
    };

    CachePtr CacheFactory(CacheType cacheType, Data cacheData);
}