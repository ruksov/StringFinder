#pragma once
#include <functional>
#include <memory>

namespace sf::lib::diff_cache
{
    struct DiffInfo
    {
        uint32_t Offset = 0;
        char Byte = 0;

        DiffInfo(uint32_t offset, char byte)
            : Offset(offset)
            , Byte(byte)
        {
        }
    };

    struct Key
    {
        union
        {
            DiffInfo Info;
            size_t CompareValue = 0;
        };

        Key(uint32_t diffOffset, char diffByte) noexcept
            : Info(diffOffset, diffByte)
        {
        }

        bool operator==(const Key& other) const noexcept
        {
            return (Info.Byte == other.Info.Byte
                && Info.Offset == other.Info.Offset);
        }

        bool operator<(const Key& other) const noexcept
        {
            return CompareValue < other.CompareValue;
        }
    };

    struct KeyHash
    {
        size_t operator()(Key const& k) const noexcept
        {
            const size_t h1 = std::hash<char>()(k.Info.Byte);
            const size_t h2 = std::hash<uint32_t>()(k.Info.Offset);
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