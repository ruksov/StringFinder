#include "stdafx.h"
#include "CacheFactory.h"
#include "Exceptions.h"
#include "DiffCache.h"

namespace sf::lib
{
    CachePtr CacheFactory(CacheType cacheType, Data cacheData)
    {
        CachePtr cache;

        switch (cacheType)
        {
        case CacheType::DiffCache:
            cache = std::make_unique<DiffCache>(std::move(cacheData));
            break;

        case CacheType::Unknown:
        default:
            THROW("Failed to create cache. Unknown cache type.")
            break;
        }

        return cache;
    }
}