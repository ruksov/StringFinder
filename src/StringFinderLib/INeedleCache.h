#pragma once
#include <vector>

namespace sf::lib
{
    using OffsetList = std::vector<size_t>;
    using Data = std::string;

    struct INeedleCache
    {
        INeedleCache() = default;
        virtual ~INeedleCache() = default;

        INeedleCache(const INeedleCache&) = delete;
        INeedleCache& operator=(const INeedleCache&) = delete;

        INeedleCache(INeedleCache&&) = delete;
        INeedleCache& operator=(INeedleCache&&) = delete;

        virtual const OffsetList& GetOffsetList(char c) const = 0;
        virtual const Data& GetNeedle() const = 0;
    };
}