#pragma once
#include "Result.h"
#include "DiffCache.h"

namespace sf::lib
{
    using Data = std::string;

    class LinearMatcher
    {
    public:
        LinearMatcher(size_t threshold, std::wstring filePath);

        size_t Match(size_t hsIndex, size_t hsOffset, const Data& hs);
        const ResultList& GetResults() const noexcept;

    private:
        size_t CompareData(size_t nlOffset, size_t hsOffset, const Data& hs);

        std::optional<Result> MatchHaystackBegin(const Data& hs);
        std::optional<Result> MatchHaystackMiddle(size_t hsOffset, const Data& hs);
        bool MatchHaystackEnd(size_t hsOffset, const Data& hs);

        void PushToResults(size_t hsIndex, size_t hsSize, const Result& res, bool isCombineResult);

    private:
        size_t m_threshold;
        ResultList m_results;
        std::map<size_t, Result> m_combineResults;
        std::pair<size_t, diff_cache::Iterator> m_prevRes;
        Data m_needle;
        diff_cache::DiffCache m_cache;

    };
}


