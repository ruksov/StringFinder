#include "stdafx.h"
#include "LinearMatcher.h"
#include "Log.h"

namespace sf::lib
{
    constexpr size_t OneMb = 1'000'000;

    LinearMatcher::LinearMatcher(size_t threshold, std::wstring filePath)
        : m_threshold(threshold)
        , m_cache(std::make_unique<DiffCacheWrapper>(std::move(filePath)))
    {
        m_resultLog.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        m_resultLog.open("result.log", std::ios::out);
    }

    size_t LinearMatcher::Match(size_t hsIndex, size_t hsOffset, const Data & hs)
    {
        {
            // When range between hsOffset and haystacks end less than threshold,
            // run specific match function to try find first part for combine result
            const auto bytesToTheEnd = hs.size() - hsOffset;
            if (bytesToTheEnd < m_threshold)
            {
                return MatchHaystackEnd(hsOffset, hs) ? bytesToTheEnd : 0;
            }
        }

        std::optional<Result> res;

        if (hsOffset == 0)
        {
            res = MatchHaystackBegin(hs);
        }
        else
        {
            res = MatchHaystackMiddle(hsOffset, hs);
        }

        if (!res)
        {
            return 0;
        }

        const bool isCombineResult = hsOffset != res.value().HsOffset;
        PushToResults(hsIndex, hs.size(), res.value(), isCombineResult);
        if (isCombineResult)
        {
            res.value().MatchLen -= hs.size() - (res.value().HsOffset % hs.size());
        }
        return res.value().MatchLen;
    }

    std::optional<Result> LinearMatcher::MatchHaystackBegin(const Data & hs)
    {
        Result maxRes;
        auto res = m_cache->CompareFirst(0, hs);

        for (;;)
        {
            if (!res)
            {
                break;
            }


            if (res->MatchLen > maxRes.MatchLen)
            {
                maxRes = *res;
            }

            // try find first part in previous results
            auto& subStrings = m_cache->GetSubStrings(res->NlOffset);
            for (auto subOffset : subStrings)
            {
                auto combIt = m_combineResults.find(subOffset);
                if (combIt != m_combineResults.end()
                    && combIt->second.MatchLen + m_cache->GetCacheDataSize() - subOffset >= maxRes.MatchLen)
                {
                    maxRes = combIt->second;
                    maxRes.MatchLen += res->MatchLen;
                }
            }

            auto combIt = m_combineResults.find(res->NlOffset);
            if (combIt != m_combineResults.end()
                && combIt->second.MatchLen + res->MatchLen >= maxRes.MatchLen)
            {
                maxRes = combIt->second;
                maxRes.MatchLen += res->MatchLen;
            }

            res = m_cache->CompareNext(res->NlOffset, res->MatchLen, 0, hs);
        }
        
        m_combineResults.clear();
        if (maxRes.MatchLen < m_threshold)
        {
            return std::nullopt;
        }
        return maxRes;
    }

    std::optional<Result> LinearMatcher::MatchHaystackMiddle(size_t hsOffset, const Data & hs)
    {
        Result maxRes;
        auto res = m_cache->CompareFirst(hsOffset, hs);

        for (;;)
        {
            if (!res)
            {
                break;
            }

            maxRes = *res;
            res = m_cache->CompareNext(res->NlOffset, res->MatchLen, hsOffset, hs);
        }

        if (maxRes.MatchLen < m_threshold)
        {
            return std::nullopt;
        }
        return maxRes;
    }

    bool LinearMatcher::MatchHaystackEnd(size_t hsOffset, const Data & hs)
    {
        assert(m_combineResults.empty());
        assert(hs.size() - hsOffset < m_threshold);

        auto res = m_cache->CompareFirst(hsOffset, hs);

        for (;;)
        {
            if (!res)
            {
                break;
            }

            if (hsOffset + res->MatchLen == hs.size())
            {
                m_combineResults.emplace(res->NlOffset + res->MatchLen, *res);

                auto& subStrings = m_cache->GetSubStrings(res->NlOffset);
                auto subStringsEnd = std::upper_bound(subStrings.begin(),
                    subStrings.end(),
                    m_cache->GetCacheDataSize() - res->MatchLen);

                for (auto subOffsetIt = subStrings.begin(); subOffsetIt != subStringsEnd; ++subOffsetIt)
                {
                    m_combineResults.emplace(*subOffsetIt + res->MatchLen,
                        Result(res->HsOffset, *subOffsetIt, res->MatchLen));
                }

                break;
            }

            res = m_cache->CompareNext(res->NlOffset, res->MatchLen, hsOffset, hs);
        }

        return !m_combineResults.empty();
    }

    void LinearMatcher::PushToResults(size_t hsIndex, size_t hsSize, Result& res, bool isCombineResult)
    {
        if (res.HsOffset + res.MatchLen == hsSize)
        {
            LOG_DEBUG("Save result, which matches to end of hs data chunck");
            m_cacheRes = res;
            m_cacheRes.HsOffset += hsIndex * hsSize;
            return;
        }

        if (m_cacheRes.MatchLen != 0
            && m_cacheRes.NlOffset + m_cacheRes.MatchLen == res.NlOffset
            && m_cacheRes.HsOffset > m_cache->GetCacheDataSize() * (hsIndex - 1))
        {
            assert(hsIndex != 0);
            res.MatchLen += m_cacheRes.MatchLen;
            res.HsOffset = m_cacheRes.HsOffset;
            res.NlOffset = m_cacheRes.NlOffset;
            assert(res.NlOffset + res.MatchLen < m_needle.size());

            LOG_DEBUG("Found match result wich is continue of previous.\n"
                << "Fixed match result:\n"
                << "\tHsOffset = " << res.HsOffset << '\n'
                << "\tNlOffset = " << res.NlOffset << '\n'
                << "\tMatchLen = " << res.MatchLen << '\n');
        }
        else
        {
            if (m_cacheRes.MatchLen != 0)
            {
                m_resultLog << "sequence of length = " << m_cacheRes.MatchLen
                    << " found at haystack offset " << m_cacheRes.HsOffset
                    << ", needle offset " << m_cacheRes.NlOffset << '\n';
            }
            
            res.HsOffset += m_cache->GetCacheDataSize() * (hsIndex - (isCombineResult ? 1 : 0));
            
            LOG_DEBUG("Found new match result:\n"
                << "\tHsOffset = " << res.HsOffset << '\n'
                << "\tNlOffset = " << res.NlOffset << '\n'
                << "\tMatchLen = " << res.MatchLen << '\n');
        }

        m_resultLog << "sequence of length = " << res.MatchLen
            << " found at haystack offset " << res.HsOffset
            << ", needle offset " << res.NlOffset << '\n';

        m_cacheRes.MatchLen = 0;
    }
}
