#include "stdafx.h"
#include "StringFinder.h"

#include "FileReader.h"
#include "Exceptions.h"
#include "Log.h"
#include "Matcher.h"

#include "ProgressBarCollection.h"
#include "ConsoleProgressBar.h"

namespace sf
{
    void StringFinder::Run(size_t threshold, std::string needlePath, std::string haystackPath)
    {
        m_resultLog.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        m_resultLog.open("result.log", std::ios::out);

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
        //m_matcher = std::make_unique<lib::LinearMatcher>(threshold, std::move(needlePath));
        lib::Matcher matcher(threshold, std::move(needlePath), [this](lib::Result res) { PrintResult(res); });
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
            const size_t hsIndex = m_haystack->GetIndex();
            const auto hsDataSize = hsData.get().size();
            for (size_t i = 0; i < hsDataSize;)
            {
                size_t matchLen = matcher.Match(hsIndex, i, hsData);
                i += matchLen == 0 ? 1 : matchLen;
            }

            progressView.OnProgressChange(m_haystack->GetIndex());
            hsData = m_haystack->GetNext();
        }
        LOG_INFO("String Finder finished search");
    }
    void StringFinder::PrintResult(lib::Result res)
    {
        LOG_DEBUG("Found new match result:\n"
            << "\tHsOffset = " << res.HsDataOffset + (res.HsDataIndex * m_haystack->GetDataSize()) << '\n'
            << "\tNlOffset = " << res.NlOffset << '\n'
            << "\tMatchLen = " << res.MatchLen << '\n');
        m_resultLog << "sequence of length = " << res.MatchLen
            << " found at haystack offset " << res.HsDataOffset + (res.HsDataIndex * m_haystack->GetDataSize())
            << ", needle offset " << res.NlOffset << '\n';
    }
}