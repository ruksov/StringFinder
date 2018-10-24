#pragma once
#include "DoubleBuffer.h"
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
        std::unique_ptr<lib::DoubleBuffer> m_haystack;
    };
}