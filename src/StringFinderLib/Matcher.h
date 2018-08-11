#pragma once
#include "IReader.h"
#include "INeedleCache.h"

namespace sf::lib
{
    class Matcher
    {
    public:
        struct Result
        {
            uint32_t MatchLen = 0;
            uint32_t NdlOffset = 0;

            bool isValid() { return MatchLen != 0; }
        };

    public:
        Matcher(uint32_t threshold, std::unique_ptr<INeedleCache> ndlCache);

        Result TryMatch(uint32_t hsIndx, const Data& hs);

    private:
        const size_t m_threshold;
        const std::unique_ptr<INeedleCache> m_ndlCache;
    };
}