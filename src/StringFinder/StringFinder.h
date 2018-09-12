#pragma once
#include "MatchResult.h"

namespace sf
{
    class StringFinder
    {
    public:
        void Run(size_t threshold, std::string needlePath, std::string haystackPath);

        void PrintResult(const lib::MatchResult& res);

    private:
        std::ofstream m_resultLog;
        size_t m_chunckSize = 0;
    };
}