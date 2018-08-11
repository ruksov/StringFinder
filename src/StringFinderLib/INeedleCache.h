#pragma once
#include <vector>

namespace sf::lib
{
    using IndexList = std::vector<uint32_t>;

    struct INeedleCache
    {
        INeedleCache() = default;
        virtual ~INeedleCache() = default;

        INeedleCache(const INeedleCache&) = delete;
        INeedleCache& operator=(const INeedleCache&) = delete;

        INeedleCache(INeedleCache&&) = delete;
        INeedleCache& operator=(INeedleCache&&) = delete;

        virtual const IndexList& GetIndexList(char c) const = 0;
        virtual const std::string& GetNeedle() const = 0;
    };
}