#include "stdafx.h"
#include "StringFinder.h"

#include "FileReader.h"
#include "Exceptions.h"
#include "Log.h"

#include "CacheFactory.h"
#include "MatcherFactory.h"

#include "ProgressBarCollection.h"
#include "ConsoleProgressBar.h"

namespace sf
{
    void StringFinder::Run(size_t threshold, std::string needlePath, std::string haystackPath)
    {
        m_resultLog.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        m_resultLog.open("result.log", std::ios::out);

        LOG_INFO("Run String Finder");
        // Read needle
        std::ifstream file;
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        file.open(needlePath, std::ios::in | std::ios::binary);
        lib::Data needleData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        LOG_INFO("Initialize reader for haystack buffer");
        auto haystackReader = std::make_unique<lib::FileReader>(std::move(haystackPath), needleData.size());

        LOG_INFO("Initialize haystack buffer");
        m_haystack = std::make_unique<lib::DoubleBuffer>(std::move(haystackReader));

        LOG_INFO("Initialize matcher");
        auto matcher = lib::MatcherFactory(lib::MatcherType::ThreasholdMatcher
            , threshold
            , lib::CacheFactory(lib::CacheType::DiffCache, std::move(needleData)));
        LOG_INFO("Finish initialize matcher");;

        // add result handler to matcher
        auto resultHandler = std::make_shared<lib::ResultHandlerFn>();
        *resultHandler = [this](const lib::MatchResult& res) { PrintResult(res); };
        matcher->AddResultHandler(resultHandler);

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
                size_t matchLen = matcher->Match(i, hsIndex, hsData);
                i += matchLen == 0 ? 1 : matchLen;
            }

            progressView.OnProgressChange(m_haystack->GetIndex());
            hsData = m_haystack->GetNext();
        }
        LOG_INFO("String Finder finished search");
    }
    void StringFinder::PrintResult(const lib::MatchResult& res)
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