#pragma once
#include "DiffCacheTypes.h"

namespace sf::lib::diff_cache
{
    using Data = std::string;
    using DiffCachePtr = std::unique_ptr<DiffCache>;
    using IteratorList = std::vector<ConstIterator>;

    DiffCachePtr Create(const Data& data, IteratorList& iteratorList);
}