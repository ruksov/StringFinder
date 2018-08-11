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

    Matcher::Result Matcher::TryMatch(uint32_t hsIndx, const Data & hs)
    {
        Result res;
        auto indxList = m_ndlCache->GetIndexList(hs.at(hsIndx));
        if (indxList.size() != 0)
        {
            auto& needle = m_ndlCache->GetNeedle();
            for (auto ni : indxList)
            {
                // check index overlaping with result
                if (ni < res.NdlOffset + res.MatchLen)
                {
                    continue;
                }

                uint32_t matchLen = 0;
                for (uint32_t hi = hsIndx; 
                    hi < hs.size() && matchLen < needle.size() && hs.at(hi) == needle.at(matchLen); 
                    ++hi, ++matchLen)
                {
                }

                // result is max match lenth range
                if (matchLen >= m_threshold && matchLen > res.MatchLen)
                {
                    res.MatchLen = matchLen;
                    res.NdlOffset = ni;
                }
            }
        }

        return res;
    }
}