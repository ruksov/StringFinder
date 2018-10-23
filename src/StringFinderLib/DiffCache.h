#pragma once
#include "DiffCacheTypes.h"

namespace sf::lib::diff_cache
{
    using Data = std::string;
    using DiffCacheRef = std::reference_wrapper<DiffCache>;
    using IteratorList = std::vector<Iterator>;

    DiffCache Create(const Data& data, IteratorList& iteratorList);
}