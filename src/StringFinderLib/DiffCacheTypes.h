#pragma once
#include <vector>
#include <unordered_map>
#include <memory>

namespace sf::lib
{
    struct DiffCacheKey
    {
        // offset of the first different byte
        size_t DiffOffset = 0;

        // value of the first different byte
        char DiffByte = 0;


        DiffCacheKey(size_t diffOffset, char diffByte) noexcept
            : DiffOffset(diffOffset)
            , DiffByte(diffByte)
        {
        }

        bool operator==(const DiffCacheKey& other) const noexcept
        {
            return (DiffOffset == other.DiffOffset && DiffByte == other.DiffByte);
        }
    };

    struct DiffCacheKeyHash
    {
        size_t operator()(DiffCacheKey const& k) const noexcept
        {
            const size_t h1 = std::hash<char>()(k.DiffByte);
            return h1 ^ (k.DiffOffset << 1);
        }
    };

    using DiffCacheContainer = std::unordered_map<DiffCacheKey, struct DiffCacheValue, DiffCacheKeyHash>;

    struct DiffCacheValue
    {
        // offset of the first byte 
        size_t Offset;

        // parent iterator
        // if node has not parent it is equal Offset field
        size_t ParentOffset;

        // tree of ranges, which have same begin and differents starting from some byte
        std::unique_ptr<DiffCacheContainer> DiffRanges;

        explicit DiffCacheValue(size_t offset)
            : Offset(offset)
            , ParentOffset(offset)
        {
        }

        DiffCacheValue(size_t offset, size_t parentOffset)
            : Offset(offset)
            , ParentOffset(parentOffset)
        {
        }
    };
}