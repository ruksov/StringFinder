#pragma once
#include "Result.h"
#include "DiffCacheWrapper.h"

namespace sf::lib
{
    class Matcher
    {
    public:
        Matcher(size_t threshold, std::string needlePath);
        std::optional<Result> Match(size_t hsOffset, const Data& data);

    private:
        size_t CompareWithHaystack(size_t nlOffset, size_t hsOffset, const Data & hsData) const;

    private:
        size_t m_threshold;
        std::optional<Result> m_prevRes;
        Data m_needleData;
        diff_cache::DiffCachePtr m_needleCache;
        diff_cache::IteratorList m_cacheIteratorList;
    };
}
