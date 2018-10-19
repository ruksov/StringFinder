#include "stdafx.h"
#include "Matcher.h"

namespace sf::lib
{
    Matcher::Matcher(size_t threshold, std::string needlePath, LogResultFn logResultFn)
        : m_threshold(threshold)
        , m_logResultFn(logResultFn)
    {
        {
            std::ifstream file;
            file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            file.open(needlePath, std::ios::in | std::ios::binary);
            m_needleData.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        }

        m_needleCache = diff_cache::Create(m_needleData, m_needleIteratorList);
    }

    size_t Matcher::Match(size_t hsDataIndex, size_t hsDataOffset, const Data & hsData)
    {
        //std::optional<Result> res;
        if (hsDataOffset == 0)
        {
            size_t currentMatchLen = 0;
            auto res = GetResultFromPrevChunck(hsData, currentMatchLen);
            if (res && res->MatchLen >= m_threshold && m_logResultFn)
            {
                m_logResultFn(res.value());
                return currentMatchLen;
            }
        }

        auto res = FindMaxResult(hsDataOffset, hsData);

        if (!res)
        {
            return 0;
        }

        res->HsDataIndex = hsDataIndex;
        if (res->HsDataOffset + res->MatchLen == hsData.size())
        {
            m_prevRes = res;
        }
        else if (res->MatchLen < m_threshold)
        {
            res->MatchLen = 0;
        }

        if (res->MatchLen >= m_threshold && !m_prevRes && m_logResultFn)
        {
            m_logResultFn(res.value());
        }

        return res->MatchLen;
    }

    size_t Matcher::CompareWithHaystack(size_t nlOffset, size_t hsOffset, const Data & hsData) const
    {
        size_t matchLen = 0;

        for (; nlOffset + matchLen < m_needleData.size() && hsOffset + matchLen < hsData.size()
            && m_needleData.at(nlOffset + matchLen) == hsData.at(hsOffset + matchLen);
            ++matchLen);

        return matchLen;
    }

    std::optional<Result> Matcher::GetResultFromPrevChunck(const Data& hsData, size_t& currentMatchLen)
    {
        if (!m_prevRes)
        {
            return std::nullopt;
        }

        auto res = m_prevRes.value();
        m_prevRes.reset();

        currentMatchLen += CompareWithHaystack(res.NlOffset + res.MatchLen, 0, hsData);

        auto it = m_needleIteratorList.at(res.NlOffset);
        if (res.NlOffset != it->second.Offset
            || !it->second.DiffStrings
            || currentMatchLen >= hsData.size())
        {
            res.MatchLen += currentMatchLen;
            return res;
        }

        DiffCacheRef cacheRef = *it->second.DiffStrings;
        it = cacheRef.get().find(diff_cache::Key(static_cast<uint32_t>(res.MatchLen + currentMatchLen)
            , hsData.at(currentMatchLen)));

        while (it != cacheRef.get().end())
        {
            res.NlOffset = it->second.Offset;
            currentMatchLen += CompareWithHaystack(res.NlOffset + it->first.Info.Offset
                , currentMatchLen
                , hsData);

            if (it->first.Info.Offset == res.MatchLen + currentMatchLen // we did not match more bytes
                || !it->second.DiffStrings                              // we can't more move through diff tree
                || currentMatchLen >= hsData.size())                    // we matched full haysatck data
            {
                break;
            }

            cacheRef = *it->second.DiffStrings;
            it = cacheRef.get().find(diff_cache::Key(static_cast<uint32_t>(res.MatchLen + currentMatchLen)
                , hsData.at(currentMatchLen)));
        }

        res.MatchLen += currentMatchLen;
        return res;
    }

    std::optional<Result> Matcher::FindMaxResult(size_t hsOffset, const Data & hsData) const
    {
        return FindMaxResult(*m_needleCache
            , m_needleCache->find(diff_cache::Key(0, hsData.at(hsOffset)))
            , hsOffset
            , hsData);
    }

    std::optional<Result> Matcher::FindMaxResult(DiffCacheRef cache, 
        diff_cache::Iterator it, 
        size_t hsOffset, 
        const Data & hsData) const 
    {
        Result res;
        res.HsDataOffset = hsOffset;

        while (it != cache.get().end())
        {
            res.NlOffset = it->second.Offset;
            res.MatchLen = it->first.Info.Offset;

            res.MatchLen += CompareWithHaystack(it->second.Offset + res.MatchLen, hsOffset + res.MatchLen, hsData);

            if (it->first.Info.Offset == res.MatchLen                   // we did not match more bytes
                || !it->second.DiffStrings                              // we can't more move through diff tree
                || res.MatchLen + res.HsDataOffset >= hsData.size())    // we matched full haysatck data
            {
                break;
            }

            cache = *it->second.DiffStrings;
            it = cache.get().find(diff_cache::Key(static_cast<uint32_t>(res.MatchLen)
                , hsData.at(res.HsDataOffset + res.MatchLen)));
        }

        return res.MatchLen == 0 ? std::nullopt : std::make_optional(res);
    }
}