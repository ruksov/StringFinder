#pragma once
#include <vector>

namespace sf::lib
{
    struct Result
    {
        uint32_t HsOffset = 0;
        uint32_t NlOffset = 0;
        uint32_t MatchLen = 0;

        Result() = default;

        Result(uint32_t hsOffset, uint32_t nlOffset, uint32_t matchLen) noexcept
            : HsOffset(hsOffset)
            , NlOffset(nlOffset)
            , MatchLen(matchLen)
        {
        }
    };

    using ResultList = std::vector<Result>;
}