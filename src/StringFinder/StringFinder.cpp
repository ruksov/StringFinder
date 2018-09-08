#include "stdafx.h"
#include "StringFinder.h"

#include "SortedNeedleCache.h"
#include "FileReader.h"
#include "Exceptions.h"
#include "Log.h"

namespace sf
{
    void StringFinder::Run(size_t threshold, std::wstring needlePath, std::wstring haystackPath)
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

        //LOG_INFO("Initialize needle cache");
        //auto needleCache = std::make_unique<lib::SortedNeedleCache>(std::move(needlePath));

        LOG_INFO("Initialize matcher");
        m_matcher = std::make_unique<lib::LinearMatcher>(threshold, std::move(needlePath));
        LOG_INFO("Finish initialize matcher");;

        LOG_INFO("Initialize reader for haystack buffer");
        auto haystackReader = std::make_unique<lib::FileReader>(std::move(haystackPath), needleSize);

        LOG_INFO("Initialize haystack buffer");
        m_haystack = std::make_unique<lib::DoubleBuffer>(std::move(haystackReader));

        LOG_INFO("String Finder start search")
        auto hsData = m_haystack->GetNext();
        while (!hsData.get().empty())
        {
            LOG_DEBUG("Start handle haystack data #" << m_haystack->GetIndex());
            const auto hsDataSize = hsData.get().size();
            for (size_t i = 0; i < hsDataSize; ++i)
            {
                i += m_matcher->Match(m_haystack->GetIndex(), i, hsData);
            }

            hsData = m_haystack->GetNext();
        }
        LOG_INFO("String Finder finished search");
    }

    void StringFinder::PrintResults() const
    {
        THROW_IF(!m_matcher, "Before you print the result, you need to start the search");
        std::wstringstream st;
        st << "Results: ";
        
        auto& results = m_matcher->GetResults();
        if (results.empty())
        {
            st << "empty";
            LOG_INFO(st.str());
            return;
        }

        st << '\n';
        for (auto& res : results)
        {
            st << "\tsequence of length = " << res.MatchLen
                << " found at haystack offset " << res.HsOffset
                << ", needle offset " << res.NlOffset << '\n';
        }
        LOG_INFO(st.str());
    }
}