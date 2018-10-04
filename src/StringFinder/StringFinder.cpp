#include "stdafx.h"
#include "StringFinder.h"

#include "FileReader.h"
#include "Exceptions.h"
#include "Log.h"

#include "ProgressBarCollection.h"
#include "ConsoleProgressBar.h"

namespace sf
{
    void StringFinder::Run(size_t threshold, std::string needlePath, std::string haystackPath)
    {
        LOG_INFO("Run String Finder");
        // Compute needle size
        size_t needleSize = 0;
        {
            std::ifstream file;
            file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            file.open(needlePath, std::ios::in | std::ios::binary | std::ios::ate);
            needleSize = file.tellg();
        }

        LOG_INFO("Initialize matcher");
        m_matcher = std::make_unique<lib::LinearMatcher>(threshold, std::move(needlePath));
        LOG_INFO("Finish initialize matcher");;

        LOG_INFO("Initialize reader for haystack buffer");
        auto haystackReader = std::make_unique<lib::FileReader>(std::move(haystackPath), needleSize);

        LOG_INFO("Initialize haystack buffer");
        m_haystack = std::make_unique<lib::DoubleBuffer>(std::move(haystackReader));

        lib::ProgressBarCollection progressView(m_haystack->GetDataCount());
        progressView.AddCallback("console", lib::ConsoleProgressBar);
        
        LOG_INFO("String Finder start search")
        auto hsData = m_haystack->GetNext();
        while (!hsData.get().empty())
        {
            LOG_DEBUG("Start handle haystack data #" << m_haystack->GetIndex());
            const auto hsDataSize = hsData.get().size();
            for (size_t i = 0; i < hsDataSize; ++i)
            {
                size_t matchLen = m_matcher->Match(m_haystack->GetIndex(), i, hsData);
                if (matchLen != 0)
                {
                    i += matchLen - 1;
                }
            }

            progressView.OnProgressChange(m_haystack->GetIndex());
            hsData = m_haystack->GetNext();
        }
        LOG_INFO("String Finder finished search");
    }
}