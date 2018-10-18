#pragma once
#include <vector>

namespace sf::lib
{
    struct Result
    {
        size_t HsDataIndex = 0;
        size_t HsDataOffset = 0;
        size_t NlOffset = 0;
        size_t MatchLen = 0;

        Result() = default;

        Result(size_t hsOffset, size_t nlOffset, size_t matchLen) noexcept
            : HsDataOffset(hsOffset)
            , NlOffset(nlOffset)
            , MatchLen(matchLen)
        {
        }
    };

    using ResultList = std::vector<Result>;
}