#pragma once
#include "DoubleBuffer.h"
#include "LinearMatcher.h"

namespace sf
{
    class StringFinder
    {
    public:
        void Run(size_t threshold, std::string needlePath, std::string haystackPath);

    private:
        std::unique_ptr<lib::LinearMatcher> m_matcher;
        std::unique_ptr<lib::DoubleBuffer> m_haystack;
    };
}