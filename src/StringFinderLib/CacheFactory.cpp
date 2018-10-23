#include "stdafx.h"
#include "CacheFactory.h"
#include "DiffCache.h"
#include "Exceptions.h"

namespace sf::lib
{
    CachePtr CacheFactory(CacheType cacheType, Data cacheData)
    {
        CachePtr cache;

        switch (cacheType)
        {
        case sf::lib::CacheType::DiffCache:
            cache = std::make_unique<DiffCache>(std::move(cacheData));
            break;

        case sf::lib::CacheType::Unknown:
        default:
            THROW("Faile to create cache. Unknown cache type.")
            break;
        }

        return cache;
    }
}