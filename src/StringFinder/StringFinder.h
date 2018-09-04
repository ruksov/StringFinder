#pragma once
#include "DoubleBuffer.h"
#include "Matcher.h"

namespace sf
{
    class StringFinder
    {
    public:
        void Run(size_t threshold, std::wstring needlePath, std::wstring haystackPath);
        void PrintResults() const;

    private:
        std::unique_ptr<lib::Matcher> m_matcher;
        std::unique_ptr<lib::DoubleBuffer> m_haystack;
    };
}