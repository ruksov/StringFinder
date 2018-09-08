#pragma once
#include <functional>
#include <optional>

namespace sf::lib::diff_cache
{
    struct Key
    {
        char DiffChar = 0;
        size_t DiffOffset = 0;

        Key(char diffCahr, size_t diffOffset) noexcept
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
            const size_t h = std::hash<char>()(k.DiffChar);
            return h ^ (k.DiffOffset << 1);
        }
    };

    struct Value;
    using DiffCache = std::unordered_map<Key, Value, KeyHash>;

    using Iterator = DiffCache::iterator;
    using ConstIterator = DiffCache::const_iterator;
    using OffsetList = std::vector<size_t>;

    struct Value
    {
        size_t Offset;
        OffsetList SubStrings;
        DiffCache DiffStrings;
        std::optional<Iterator> NextDataByte;

        explicit Value(size_t offset) noexcept
            : Offset(offset)
        {
        }
        #pragma warning (suppress : 26495)
    };
}