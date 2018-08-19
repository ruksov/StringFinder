#include "stdafx.h"
#include "Matcher.h"
#include "Exceptions.h"

namespace sf::lib
{
    Matcher::Matcher(uint32_t threshold, std::unique_ptr<INeedleCache> ndlCache)
        : m_threshold(threshold)
        , m_ndlCache(std::move(ndlCache))
    {
    }

    Matcher::Result Matcher::TryMatch(uint32_t hsIndx, const Data & haystack)
    {
        Result maxRes;

        auto indxList = m_ndlCache->GetIndexList(haystack.at(hsIndx));
        if (indxList.size() != 0)
        {
            auto& needle = m_ndlCache->GetNeedle();

            uint32_t hsEnd = haystack.size();
            uint32_t nlEnd = needle.size();

            uint32_t ni = 0;
            uint32_t hi = 0;

            for (auto currentNi : indxList)
            {
                if (currentNi - nlEnd < m_threshold
                    || currentNi - nlEnd < maxRes.MatchLen)
                {
                    break;
                }

                ni = currentNi;
                hi = hsIndx;

                for ( ; hi < hsEnd && ni < nlEnd && haystack.at(hi) == needle.at(ni);
                    ++hi, ++ni)
                {
                }

                // compare with max result
                uint32_t matchLen = ni - currentNi;
                if (matchLen >= m_threshold && matchLen > maxRes.MatchLen)
                {
                    maxRes.MatchLen = matchLen;
                    maxRes.NdlOffset = currentNi;
                }
            }
        }

        return maxRes;
    }
}