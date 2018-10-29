#pragma once
#include "IMatcher.h"

namespace sf::lib
{
    class ThreasholdMatcher : public IMatcher
    {
    public:
        ThreasholdMatcher(size_t threashold, CachePtr cache);

        size_t Match(size_t hsOffset, size_t hsDataIndex, const Data& hsData) override;

    private:
        MatchResult GetMaxResult(size_t hsOffset,
            size_t hsDataIndex,
            const Data& hsData);

        MatchResult GetMaxResult(size_t hsOffset, 
            const MatchResult& cachedMatchRes, 
            const Data& hsData);

        MatchResult GetMaxResult_FromBegin(const MatchResult & cachedMatchRes, const Data & hsData);

    private:
        size_t m_threashold = 0;
        CachePtr m_cache;
        std::optional<MatchResult> m_cachedMatchRes;
        std::optional<MatchResult> m_matchResFromPrevChunck;
    };
}


