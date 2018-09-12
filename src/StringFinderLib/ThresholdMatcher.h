#pragma once
#include "IMatcher.h"

namespace sf::lib
{
    class ThresholdMatcher : public IMatcher
    {
    public:
        ThresholdMatcher(size_t threashold, CachePtr cache);

        size_t Match(size_t hsOffset, size_t hsDataIndex, const Data& hsData) override;

    private:
        void GetMaxResult(size_t hsOffset, MatchResult& inOutRes, const Data& hsData);

    private:
        size_t m_threshold = 0;
        CachePtr m_cache;
        size_t m_cacheSize = 0;
        std::optional<MatchResult> m_cachedMatchRes;
    };
}


