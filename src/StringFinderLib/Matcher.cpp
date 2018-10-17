#include "stdafx.h"
#include "Matcher.h"

namespace sf::lib
{
    Matcher::Matcher(size_t threshold, std::string needlePath)
        : m_threshold(threshold)
    {
        {
            std::ifstream file;
            file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            file.open(needlePath, std::ios::in | std::ios::binary);
            m_needleData.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        }

        m_needleCache = diff_cache::Create(m_needleData, m_cacheIteratorList);
    }

    std::optional<Result> Matcher::Match(size_t hsOffset, const Data & data)
    {
        std::optional<Result> res;
        diff_cache::Iterator it;

        if (m_prevRes)
        {
            size_t cmpNlOffset = m_prevRes->NlOffset + m_prevRes->MatchLen;
            size_t cmpHsOffset = hsOffset == 0 ? hsOffset : m_prevRes->HsOffset + m_prevRes->MatchLen;

            m_prevRes->MatchLen += CompareWithHaystack(cmpNlOffset, cmpHsOffset, data);

            if (m_prevRes->NlOffset + m_prevRes->MatchLen < m_needleData.size())
            {
                it = m_needleCache->find(diff_cache::Key(static_cast<uint32_t>(m_prevRes->MatchLen),
                    m_needleData.at(m_prevRes->NlOffset + m_prevRes->MatchLen)));
            }
        }
        else
        {
            it = m_needleCache->find(diff_cache::Key(0, data.at(hsOffset)));
        }

        while (it != m_needleCache->end())
        {
            size_t diffOffset = it->first.Info.Offset;
            size_t cmpNlOffset = it->second.Offset + diffOffset;
            size_t cmpHsOffset = hsOffset + diffOffset;

            size_t matchLen = CompareWithHaystack(cmpNlOffset, cmpHsOffset, data);

            if (matchLen == 0)
            {
                break;
            }

            res = Result(hsOffset, it->second.Offset, diffOffset + matchLen);

            if (cmpHsOffset + matchLen < data.size())
            {
                it = m_needleCache->find(diff_cache::Key(static_cast<uint32_t>(diffOffset + matchLen), 
                    data.at(cmpHsOffset + matchLen)));
            }
            else
            {
                it = m_needleCache->end();
            }
        }

        if (res && hsOffset + res->MatchLen == data.size())
        {
            m_prevRes = res;
        }
        else if (res && res->MatchLen < m_threshold)
        {
            if (res->MatchLen > 2)
            {
                m_prevRes = Result(res->HsOffset + 1, res->NlOffset + 1, res->MatchLen - 1);
            }
            else
            {
                m_prevRes.reset();
            }
            res.reset();
        }
        else if (!res && m_prevRes && m_prevRes->MatchLen >= m_threshold)
        {
            res = m_prevRes;
            m_prevRes.reset();
        }

        return res;
    }

    size_t Matcher::CompareWithHaystack(size_t nlOffset, size_t hsOffset, const Data & hsData) const
    {
        size_t matchLen = 0;

        for (; nlOffset + matchLen < m_needleData.size() && hsOffset + matchLen < hsData.size()
            && m_needleData.at(nlOffset + matchLen) == hsData.at(hsOffset + matchLen);
            ++matchLen);

        return matchLen;
    }
}