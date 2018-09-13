#include "stdafx.h"
#include "LinearMatcher.h"
#include "Log.h"

namespace sf::lib
{
    constexpr size_t OneMb = 1'000'000;

    LinearMatcher::LinearMatcher(size_t threshold, std::wstring filePath)
        : m_threshold(threshold)
    {
        std::ifstream file;
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        file.open(filePath, std::ios::in | std::ios::binary);

        m_resultLog.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        m_resultLog.open("result.log", std::ios::out);

        m_needle.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        diff_cache::IteratorList itList;
        m_cache = diff_cache::Create(m_needle, itList);
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

    size_t LinearMatcher::CompareData(size_t nlOffset, size_t hsOffset, const Data & hs)
    {
        size_t matchLen = 0;
        const size_t nlSize = m_needle.size();
        const size_t hsSize = hs.size();

        for (; nlOffset + matchLen < nlSize && hsOffset + matchLen < hsSize
            && hs.at(hsOffset + matchLen) == m_needle.at(nlOffset + matchLen);
            ++matchLen);

        return matchLen;
    }

    std::optional<Result> LinearMatcher::MatchHaystackBegin(const Data & hs)
    {
        Result maxRes;
        auto it = m_cache->find(diff_cache::Key(0, hs.at(0)));
        auto cache = std::ref(m_cache);

        size_t matchLen = 0;

        while (it != cache.get()->end())
        {
            Result res;

            matchLen += CompareData(it->second.Offset + matchLen, matchLen, hs);
            res.MatchLen = matchLen;
            res.NlOffset = it->second.Offset;

            auto combIt = m_combineResults.find(it->second.Offset);
            if (combIt != m_combineResults.end())
            {
                res = combIt->second;
                res.MatchLen += matchLen;
            }

            if (res.MatchLen >= m_threshold && res.MatchLen > maxRes.MatchLen)
            {
                maxRes = res;
            }

            if (it->second.SubStrings)
            {
                for (auto subStrOffset : *it->second.SubStrings)
                {
                    combIt = m_combineResults.find(subStrOffset);
                    if (combIt == m_combineResults.end())
                    {
                        continue;
                    }

                    res = combIt->second;
                    const size_t bytesToEnd = m_needle.size() - subStrOffset;
                    const size_t fixedMatchLen = matchLen < bytesToEnd ? matchLen : bytesToEnd;
                    res.MatchLen += fixedMatchLen;

                    if (res.MatchLen >= m_threshold && res.MatchLen > maxRes.MatchLen)
                    {
                        maxRes = res;
                    }
                }
            }

            if (matchLen == hs.size())
            {
                break;
            }

            if (!it->second.DiffStrings)
            {
                break;
            }

            cache = std::ref(it->second.DiffStrings);
            it = cache.get()->find(diff_cache::Key(static_cast<uint32_t>(matchLen)
                , hs.at(matchLen)));
        }

        m_combineResults.clear();
        return maxRes.MatchLen == 0 ? std::nullopt : std::make_optional(maxRes);
    }

    std::optional<Result> LinearMatcher::MatchHaystackMiddle(size_t hsOffset, const Data & hs)
    {
        Result res;
        auto it = m_cache->find(diff_cache::Key(0, hs.at(hsOffset)));
        auto cache = std::ref(m_cache);
        size_t matchLen = 0;

        while (it != cache.get()->end())
        {
            matchLen += CompareData(it->second.Offset + matchLen, hsOffset + matchLen, hs);

            if (matchLen >= m_threshold && matchLen > res.MatchLen)
            {
                res.HsOffset = hsOffset;
                res.NlOffset = it->second.Offset;
                res.MatchLen = matchLen;
            }

            if (matchLen == hs.size())
            {
                break;
            }

            if (!it->second.DiffStrings)
            {
                // no more strings in diff tree
                break;
            }

            cache = std::ref(it->second.DiffStrings);
            it = cache.get()->find(diff_cache::Key(static_cast<uint32_t>(matchLen)
                , hs.at(matchLen)));
        }

        return res.MatchLen == 0 ? std::nullopt : std::make_optional(res);
    }

    bool LinearMatcher::MatchHaystackEnd(size_t hsOffset, const Data & hs)
    {
        assert(m_combineResults.empty());
        assert(hs.size() - hsOffset < m_threshold);
        auto it = m_cache->find(diff_cache::Key(0, hs.at(hsOffset)));
        auto cache = std::ref(m_cache);
        size_t matchLen = 0;

        while (it != cache.get()->end())
        {
            matchLen += CompareData(it->second.Offset + matchLen, hsOffset + matchLen, hs);

            if (hs.size() == hsOffset + matchLen)
            {
                m_combineResults.emplace(it->second.Offset + matchLen
                    , Result(hsOffset, it->second.Offset, matchLen));

                if (it->second.SubStrings)
                {
                    auto subOffsetEnd = std::lower_bound(it->second.SubStrings->begin()
                        , it->second.SubStrings->end()
                        , m_needle.size() - matchLen);

                    for (auto subOffsetIt = it->second.SubStrings->begin()
                        ; subOffsetIt != subOffsetEnd
                        ; ++subOffsetIt)
                    {
                        m_combineResults.emplace(*subOffsetIt + matchLen
                            , Result(hsOffset, *subOffsetIt, matchLen));
                    }
                }

                break;
            }

            if (!it->second.DiffStrings)
            {
                // no more strings in diff tree
                break;
            }

            cache = std::ref(it->second.DiffStrings);
            it = cache.get()->find(diff_cache::Key(static_cast<uint32_t>(matchLen)
                , hs.at(matchLen)));
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
            && m_cacheRes.HsOffset > m_needle.size() * (hsIndex - 1))
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
            
            res.HsOffset += m_needle.size() * (hsIndex - (isCombineResult ? 1 : 0));
            
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
