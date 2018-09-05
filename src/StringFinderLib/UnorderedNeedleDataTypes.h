#pragma once
#include <functional>

namespace sf::lib
{
    struct NeedleKey
    {
        char DiffChar = 0;
        size_t DiffOffset = 0;

        NeedleKey(char diffCahr, size_t diffIndex) noexcept
            : DiffChar(diffCahr)
            , DiffOffset(diffIndex)
        {
        }

        bool operator==(const NeedleKey& other) const noexcept
        {
            return (DiffChar == other.DiffChar
                && DiffOffset == other.DiffOffset);
        }
    };

    struct NeedleKeyHash
    {
        size_t operator()(NeedleKey const& k) const noexcept
        {
            const size_t h = std::hash<char>()(k.DiffChar);
            return h ^ (k.DiffOffset << 1);
        }
    };

    struct NeedleValue;
    using NeedleStrHashTable = std::unordered_map<NeedleKey, NeedleValue, NeedleKeyHash>;

    struct NeedleValue
    {
        size_t Offset;
        std::vector<size_t> SubStringOffsets;
        NeedleStrHashTable DiffStrings;

        explicit NeedleValue(size_t offset) noexcept
            : Offset(offset)
        {
        }
#pragma warning (suppress : 26495)
    };
}