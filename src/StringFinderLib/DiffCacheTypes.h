#pragma once
#include <functional>
#include <memory>

namespace sf::lib::diff_cache
{
    struct Key
    {
        uint32_t DiffOffset = 0;
        char DiffChar = 0;

        Key(char diffCahr, uint32_t diffOffset) noexcept
            : DiffChar(diffCahr)
            , DiffOffset(diffOffset)
        {
        }

        bool operator==(const Key& other) const noexcept
        {
            return (DiffChar == other.DiffChar
                && DiffOffset == other.DiffOffset);
        }
    };

    struct KeyHash
    {
        size_t operator()(Key const& k) const noexcept
        {
            const size_t h1 = std::hash<char>()(k.DiffChar);
            const size_t h2 = std::hash<uint32_t>()(k.DiffOffset);
            return h1 ^ (h2 << 1);
        }
    };

    struct Value;
    using DiffCache = std::unordered_map<Key, Value, KeyHash>;

    using Iterator = DiffCache::iterator;
    using ConstIterator = DiffCache::const_iterator;
    using OffsetList = std::vector<uint32_t>;

    struct Value
    {
        std::unique_ptr<DiffCache> DiffStrings;
        std::unique_ptr<OffsetList> SubStrings;
        std::unique_ptr<Iterator> NextDataByte;
        uint32_t Offset;

        explicit Value(uint32_t offset) noexcept
            : Offset(offset)
        {
        }
        //#pragma warning (suppress : 26495)
    };
}