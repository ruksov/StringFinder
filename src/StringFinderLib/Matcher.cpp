#include "stdafx.h"
#include "Matcher.h"
#include "Log.h"
#include "Exceptions.h"

namespace sf::lib
{
    Matcher::Matcher(size_t threshold, NeedleCachePtr needleCache)
        : m_threshold(threshold)
        , m_needleCache(std::move(needleCache))
    {
        THROW_IF(m_threshold == 0, "The threashold can't be zero");
    }

    size_t sf::lib::Matcher::Match(size_t hsIndex, size_t hsOffset, const Data& hs)
    {
        {
            const auto bytesToTheEnd = hs.size() - hsOffset;
            if ( bytesToTheEnd < m_threshold && MatchHaystackEnd(hsOffset, hs))
            {
                return bytesToTheEnd;
            }
        }
        
        Result res;

        if (hsOffset == 0)
        {
            res = MatchHaystackBegin(hs);
        }
        else
        {
            res = MatchHaystack(hsOffset, hs);
        }

        bool isCombineResult = hsOffset != res.HsOffset;
        if (res.MatchLen != 0)
        {
            PushToResults(hsIndex, hs.size(), res, isCombineResult);
            if (isCombineResult)
            {
                res.MatchLen -= hs.size() - (res.HsOffset % hs.size());
            }
        }

        return res.MatchLen;
    }

    const ResultList & Matcher::GetResults() const noexcept
    {
        return m_results;
    }

    Result Matcher::CompareData(size_t nlOffset, const Data& nl, size_t hsOffset, const Data& hs)
    {
        size_t matchLen = 0;
        const auto hsEnd = hs.size();
        const auto nlEnd = nl.size();

        for (auto ni = nlOffset, hi = hsOffset;
            hi < hsEnd && ni < nlEnd && hs.at(hi) == nl.at(ni);
            ++ni, ++hi, ++matchLen);

        return Result(hsOffset, nlOffset, matchLen);
    }

    Result Matcher::MatchHaystack(size_t hsOffset, const Data & hs)
    {
        assert(hsOffset != 0 && hs.size() - hsOffset >= m_threshold);
        THROW_IF(hsOffset >= hs.size(), "Haystack offset for matcher is bigger than haystack chunck size.");

        Result maxRes;
        auto& offsetList = m_needleCache->GetOffsetList(hs.at(hsOffset));
        auto& needle = m_needleCache->GetNeedle();
        const auto nlSize = needle.size();

        for (auto nlOffset : offsetList)
        {
            if (nlSize - nlOffset < m_threshold || nlSize - nlOffset < maxRes.MatchLen)
            {
                break;
            }

            const auto res = CompareData(nlOffset, needle, hsOffset, hs);

            if (res.MatchLen >= m_threshold && res.MatchLen > maxRes.MatchLen)
            {
                maxRes = res;
            }
        }

        return maxRes;
    }

    Result Matcher::MatchHaystackBegin(const Data & hs)
    {
        Result maxRes;
        auto& offsetList = m_needleCache->GetOffsetList(hs.front());
        auto& needle = m_needleCache->GetNeedle();
        const auto nlSize = needle.size();

        for (auto nlOffset : offsetList)
        {
            const auto it = m_combineResults.find(nlOffset);

            if (it == m_combineResults.end() &&
                (nlSize - nlOffset < m_threshold || nlSize - nlOffset < maxRes.MatchLen))
            {
                break;
            }

            auto res = CompareData(nlOffset, needle, 0, hs);
            if (it != m_combineResults.end())
            {
                res.MatchLen += it->second.MatchLen;
                res.HsOffset = it->second.HsOffset;
                res.NlOffset = it->second.NlOffset;
            }

            if (res.MatchLen >= m_threshold && res.MatchLen > maxRes.MatchLen)
            {
                maxRes = res;
            }
        }

        if (!m_combineResults.empty())
        {
            m_combineResults.clear();
        }

        return maxRes;
    }

    bool Matcher::MatchHaystackEnd(size_t hsOffset, const Data & hs)
    {
        assert(hs.size() - hsOffset < m_threshold);
        assert(m_combineResults.empty());
        THROW_IF(hsOffset >= hs.size(), "Haystack offset for matcher is bigger than haystack chunck size.");

        auto& offsetList = m_needleCache->GetOffsetList(hs.at(hsOffset));
        auto& needle = m_needleCache->GetNeedle();
        const auto hsSize = hs.size();

        for (auto nlOffset : offsetList)
        {
            auto res = CompareData(nlOffset, needle, hsOffset, hs);
            if (hsOffset + res.MatchLen == hsSize)
            {
                m_combineResults.emplace(nlOffset + res.MatchLen, res);
            }
        }
        
        return !m_combineResults.empty();
    }

    void Matcher::PushToResults(size_t hsIndex, size_t hsSize, const Result & res, bool isCombineResult)
    {
        if (!m_results.empty())
        {
            auto& prevRes = m_results.back();
            if (prevRes.NlOffset + prevRes.MatchLen == res.NlOffset)
            {
                prevRes.MatchLen += res.MatchLen;

                LOG_DEBUG("Found match result wich is continue of previous.\n"
                    << "Fixed match result:\n"
                    << "\tHsOffset = " << prevRes.HsOffset << '\n'
                    << "\tNlOffset = " << prevRes.NlOffset << '\n'
                    << "\tMatchLen = " << prevRes.MatchLen << '\n');

                return;
            }
        }

        const auto fixedHsOffset = res.HsOffset + hsSize * (hsIndex - (isCombineResult ? 1 : 0));

        LOG_DEBUG("Found new match result:\n"
            << "\tHsOffset = " << fixedHsOffset << '\n'
            << "\tNlOffset = " << res.NlOffset << '\n'
            << "\tMatchLen = " << res.MatchLen << '\n');
        m_results.emplace_back(fixedHsOffset, res.NlOffset, res.MatchLen);
    }
}