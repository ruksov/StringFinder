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
            size_t matchLenInCurrentChunck = 0;
            auto prevRes = GetResultFromPrevChunck(hsData, matchLenInCurrentChunck);
            if (prevRes.MatchLen >= m_threshold && m_logResultFn)
            {
                m_logResultFn(prevRes);
                return matchLenInCurrentChunck;
            }
        }

        Result res;
        std::reference_wrapper<diff_cache::DiffCache> cacheRef = *m_needleCache;
        auto it = cacheRef.get().find(diff_cache::Key(0, hsData.at(hsDataOffset)));

        while(it != cacheRef.get().end())
        {
            size_t diffOffset = it->first.Info.Offset;
            size_t cmpNlOffset = it->second.Offset + diffOffset;
            size_t cmpHsOffset = hsDataOffset + diffOffset;

            size_t matchLen = CompareWithHaystack(cmpNlOffset, cmpHsOffset, hsData);

            if (matchLen == 0)
            {
                break;
            }

            if (!it->second.DiffStrings || hsDataOffset + diffOffset + matchLen >= hsData.size())
            {
                res.HsDataIndex = hsDataIndex;
                res.HsDataOffset = hsDataOffset;
                res.NlOffset = it->second.Offset;
                res.MatchLen = diffOffset + matchLen;
                break;
            }

            cacheRef = *it->second.DiffStrings;
            it = cacheRef.get().find(diff_cache::Key(static_cast<uint32_t>(diffOffset + matchLen), 
                hsData.at(hsDataOffset + diffOffset + matchLen)));

            if (it == cacheRef.get().end())
            {
                res.HsDataIndex = hsDataIndex;
                res.HsDataOffset = hsDataOffset;
                res.NlOffset = cmpNlOffset - diffOffset;
                res.MatchLen = diffOffset + matchLen;
                break;
            }
        }

        if (res.HsDataOffset + res.MatchLen == hsData.size())
        {
            m_prevRes = res;
        }
        else if (res.MatchLen < m_threshold)
        {
            res.MatchLen = 0;
        }

        if (res.MatchLen >= m_threshold && !m_prevRes && m_logResultFn)
        {
            m_logResultFn(res);
        }

        return res.MatchLen;
    }

    size_t Matcher::CompareWithHaystack(size_t nlOffset, size_t hsOffset, const Data & hsData) const
    {
        size_t matchLen = 0;

        for (; nlOffset + matchLen < m_needleData.size() && hsOffset + matchLen < hsData.size()
            && m_needleData.at(nlOffset + matchLen) == hsData.at(hsOffset + matchLen);
            ++matchLen);

        return matchLen;
    }

    Result Matcher::GetResultFromPrevChunck(const Data& hsData, size_t& matchLenInCurrentChunck)
    {
        Result res;
        if (m_prevRes)
        {
            matchLenInCurrentChunck += CompareWithHaystack(m_prevRes->NlOffset + m_prevRes->MatchLen, 0, hsData);
            res = m_prevRes.value();
            res.MatchLen += matchLenInCurrentChunck;
            m_prevRes.reset();
        }
        return res;
    }
}