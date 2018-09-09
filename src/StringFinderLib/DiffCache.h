#pragma once
#include "DiffCacheTypes.h"

namespace sf::lib::diff_cache
{
    using DiffCachePtr = std::unique_ptr<DiffCache>;
    DiffCachePtr Create(const std::string& cacheData);
}