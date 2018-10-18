#include "stdafx.h"
#include "LinearMatcher.h"
#include "Log.h"

namespace sf::lib
{
    constexpr size_t OneMb = 1'000'000;

    LinearMatcher::LinearMatcher(size_t threshold, std::string filePath)
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

        const bool isCombineResult = hsOffset != res.value().HsDataOffset;
        PushToResults(hsIndex, hs.size(), res.value(), isCombineResult);
        if (isCombineResult)
        {
            res.value().MatchLen -= hs.size() - (res.value().HsDataOffset % hs.size());
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

            auto resFromPrevHsData = FindResFromPrevHsData(res->NlOffset);
            if (resFromPrevHsData)
            {
                maxRes = *resFromPrevHsData;
                maxRes.MatchLen += res->MatchLen;
            }

            if (res->NlOffset < m_cache->GetCacheDataSize() - 1
                && res->MatchLen > 1)
            {
                m_optimizedRes.NlOffset = res->NlOffset + 1;
                m_optimizedRes.MatchLen = res->MatchLen - 1;
            }
            else
            {
                m_optimizedRes.MatchLen = 0;
            }

            res = m_cache->CompareNext(res->NlOffset, res->MatchLen, 0, hs);
        }
        
        m_combineResults.clear();
        if (maxRes.MatchLen < m_threshold)
        {
            if (m_optimizedRes.MatchLen != 0)
            {
                LOG_DEBUG("Save result of comparing to optimize next iteration."
                    << "\n match length = " << m_optimizedRes.MatchLen
                    << "\n next nl offset = " << m_optimizedRes.NlOffset);
            }
            return std::nullopt;
        }

        m_optimizedRes.MatchLen = 0;
        return maxRes;
    }

    std::optional<Result> LinearMatcher::MatchHaystackMiddle(size_t hsOffset, const Data & hs)
    {
        Result maxRes;
        std::optional<Result> res;
        if (m_optimizedRes.MatchLen != 0)
        {
            LOG_DEBUG("Apply optimization, search next compare data with arguments."
                << "\n cache offset = " << m_optimizedRes.NlOffset
                << "\n match length = " << m_optimizedRes.MatchLen);
            res = m_cache->CompareNext(m_optimizedRes.NlOffset
                , m_optimizedRes.MatchLen
                , hsOffset
                , hs);

            if (!res)
            {
                m_optimizedRes.MatchLen -= 1;
                m_optimizedRes.NlOffset += 1;
            }
        }
        else
        {
            res = m_cache->CompareFirst(hsOffset, hs);
        }

        for (;;)
        {
            if (!res)
            {
                break;
            }

            maxRes = *res;
            res = m_cache->CompareNext(res->NlOffset, res->MatchLen, hsOffset, hs);
        }

        if (maxRes.NlOffset < m_cache->GetCacheDataSize() - 1
            && maxRes.MatchLen > 1)
        {
            m_optimizedRes.NlOffset = maxRes.NlOffset + 1;
            m_optimizedRes.MatchLen = maxRes.MatchLen - 1;
        }
        else
        {
            m_optimizedRes.MatchLen = 0;
        }

        if (maxRes.MatchLen < m_threshold)
        {
            if (m_optimizedRes.MatchLen != 0)
            {
                LOG_DEBUG("Save result of comparing to optimize next iteration."
                    << "\n match length = " << m_optimizedRes.MatchLen
                    << "\n next nl offset = " << m_optimizedRes.NlOffset);
            }
            return std::nullopt;
        }

        m_optimizedRes.MatchLen = 0;
        return maxRes;
#pragma warning (suppress : 26487)
    }

    bool LinearMatcher::MatchHaystackEnd(size_t hsOffset, const Data & hs)
    {
        assert(m_combineResults.empty());
        assert(hs.size() - hsOffset < m_threshold);

        std::optional<Result> res;
        if (m_optimizedRes.MatchLen != 0)
        {
            LOG_DEBUG("Apply optimization, search next compare data with arguments."
                << "\n cache offset = " << m_optimizedRes.NlOffset
                << "\n match length = " << m_optimizedRes.MatchLen);
            res = m_cache->CompareNext(m_optimizedRes.NlOffset
                , m_optimizedRes.MatchLen
                , hsOffset
                , hs);

            if (!res)
            {
                m_optimizedRes.MatchLen -= 1;
                m_optimizedRes.NlOffset += 1;
            }
        }
        else
        {
            res = m_cache->CompareFirst(hsOffset, hs);
        }

        for (;;)
        {
            if (!res)
            {
                break;
            }

            if (hsOffset + res->MatchLen == hs.size())
            {
                // we find range data, which can be continue in next chunck of haystack data
                m_combineResults.emplace(res->NlOffset + res->MatchLen, *res);

                assert(res->MatchLen < m_threshold);

                // get offsets of ranges, which equeal found range in cache
                // [sub range offset] ... [cache end] == [found range offset] ... [found range offset + sub range length]
                // and clip this list
                auto& subStrings = m_cache->GetSubStrings(res->NlOffset);               
                const auto subStringsEnd = std::upper_bound(subStrings.begin(),
                    subStrings.end(),
                    m_cache->GetCacheDataSize() - m_threshold);

#pragma warning (suppress : 26486)
                for (auto subOffsetIt = subStrings.begin(); subOffsetIt != subStringsEnd; ++subOffsetIt)
                {
                    m_combineResults.emplace(*subOffsetIt + res->MatchLen,
                        Result(res->HsDataOffset, *subOffsetIt, res->MatchLen));
                }

                m_optimizedRes.MatchLen = 0;
                break;
            }

            if (res->NlOffset < m_cache->GetCacheDataSize() - 1
                && res->MatchLen > 1)
            {
                m_optimizedRes.NlOffset = res->NlOffset + 1;
                m_optimizedRes.MatchLen = res->MatchLen - 1;
            }
            else
            {
                m_optimizedRes.MatchLen = 0;
            }

            res = m_cache->CompareNext(res->NlOffset, res->MatchLen, hsOffset, hs);
        }

        return !m_combineResults.empty();
    }

    std::optional<Result> LinearMatcher::FindResFromPrevHsData(size_t nlOffset)
    {
        auto& subRanges = m_cache->GetSubStrings(static_cast<uint32_t>(nlOffset));

        if (m_combineResults.empty()
            || nlOffset > m_combineResults.begin()->first
            || (!subRanges.empty()
                && *subRanges.begin() > m_combineResults.begin()->first))
        {
            return std::nullopt;
        }

        auto firstPart = m_combineResults.find(nlOffset);
        if (firstPart != m_combineResults.end())
        {
            return firstPart->second;
        }
        else
        {
            const auto srEnd = std::upper_bound(subRanges.begin()
                , subRanges.end()
                , m_cache->GetCacheDataSize() - m_threshold - 1);

            for (auto srIt = subRanges.begin(); srIt != srEnd; ++srIt)
            {
                firstPart = m_combineResults.find(*srIt);
                if (firstPart != m_combineResults.end())
                {
                    // Found frist part of combine result
                    LOG_DEBUG("Found first part of result from previous haystack data chunck,"
                        << " which can be summ with current result"
                        << "\n\t first part haystack offset = " << firstPart->second.HsDataOffset
                        << "\n\t first part needle offset = " << firstPart->second.NlOffset
                        << "\n\t first part match length = " << firstPart->second.MatchLen);

                    return firstPart->second;
                }
            }
        }

        return std::nullopt;
    }

    void LinearMatcher::PushToResults(size_t hsIndex, size_t hsSize, Result& res, bool isCombineResult)
    {
        if (res.HsDataOffset + res.MatchLen == hsSize)
        {
            LOG_DEBUG("Save result, which matches to end of hs data chunck");
            m_cacheRes = res;
            m_cacheRes.HsDataOffset += hsIndex * hsSize;
            return;
        }

        if (m_cacheRes.MatchLen != 0
            && m_cacheRes.NlOffset + m_cacheRes.MatchLen == res.NlOffset
            && m_cacheRes.HsDataOffset > m_cache->GetCacheDataSize() * (hsIndex - 1))
        {
            assert(hsIndex != 0);
            res.MatchLen += m_cacheRes.MatchLen;
            res.HsDataOffset = m_cacheRes.HsDataOffset;
            res.NlOffset = m_cacheRes.NlOffset;

            LOG_DEBUG("Found match result wich is continue of previous.\n"
                << "Fixed match result:\n"
                << "\tHsOffset = " << res.HsDataOffset << '\n'
                << "\tNlOffset = " << res.NlOffset << '\n'
                << "\tMatchLen = " << res.MatchLen << '\n');
        }
        else
        {
            if (m_cacheRes.MatchLen != 0)
            {
                m_resultLog << "sequence of length = " << m_cacheRes.MatchLen
                    << " found at haystack offset " << m_cacheRes.HsDataOffset
                    << ", needle offset " << m_cacheRes.NlOffset << '\n';
            }
            
            res.HsDataOffset += m_cache->GetCacheDataSize() * (hsIndex - (isCombineResult ? 1 : 0));
            
            LOG_DEBUG("Found new match result:\n"
                << "\tHsOffset = " << res.HsDataOffset << '\n'
                << "\tNlOffset = " << res.NlOffset << '\n'
                << "\tMatchLen = " << res.MatchLen << '\n');
        }

        m_resultLog << "sequence of length = " << res.MatchLen
            << " found at haystack offset " << res.HsDataOffset
            << ", needle offset " << res.NlOffset << '\n';

        m_cacheRes.MatchLen = 0;
    }
}
