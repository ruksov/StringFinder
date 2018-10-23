#pragma once
#include <vector>
#include <map>
#include <memory>

namespace sf::lib::diff_cache
{
    struct Key
    {
        // offset of the first different byte
        size_t DiffOffset = 0;

        // value of the first different byte
        char DiffByte = 0;


        Key(size_t diffOffset, char diffByte) noexcept
            : DiffOffset(diffOffset)
            , DiffByte(diffByte)
        {
        }

        bool operator==(const Key& other) const noexcept
        {
            return (DiffOffset == other.DiffOffset && DiffByte == other.DiffByte);
        }
    };

    struct KeyHash
    {
        size_t operator()(Key const& k) const noexcept
        {
            const size_t h1 = std::hash<char>()(k.DiffByte);
            const size_t h2 = k.DiffOffset;
            return (h1 << 32) ^ h2;
        }
    };

    using DiffCache = std::unordered_map<Key, struct Value, KeyHash>;
    using Iterator = DiffCache::iterator;
    using ConstIterator = DiffCache::const_iterator;
    using OffsetList = std::vector<size_t>;

    struct Value
    {
        // offset of the first byte 
        size_t Offset = 0;

        // tree of ranges, which have same begin and differents starting from some byte
        std::unique_ptr<DiffCache> DiffRanges;

        explicit Value(size_t offset) noexcept
            : Offset(offset)
        {
        }
    };
}