#include "stdafx.h"
#include "UnorderedNeedleCache.h"

using namespace testing;

namespace
{
    struct TestUnorderedNeedleCache : Test
    {
    public:
        size_t GetFileSize()
        {
            m_file.seekp(0, m_file.end);
            return m_file.tellp();
        }

        void FillFile()
        {
            m_file.open(m_path, std::ios::binary | std::ios::out);
            ASSERT_TRUE(m_file.is_open());
            m_file << m_needleData;
            m_file.close();
            m_isFileExist = true;
        }

        void ResetNeedleCache()
        {
            m_cache = std::make_unique<sf::lib::UnorderedNeedleCache>(m_path);
        }

    protected:
        void TearDown() override
        {
            if (m_isFileExist)
            {
                ASSERT_EQ(0, _wremove(m_path.c_str()));
            }
        }

    protected:
        std::ofstream m_file;
        std::string m_needleData;
        std::wstring m_path = L"test.txt";
        bool m_isFileExist = false;
        std::unique_ptr<sf::lib::UnorderedNeedleCache> m_cache;
    };
}

TEST_F(TestUnorderedNeedleCache, TestCompare_OneResult)
{
    m_needleData = "needeneedlened";
    FillFile();
    ASSERT_NO_THROW(ResetNeedleCache());

    std::vector<sf::lib::Result> results;
    auto resultCb = [&results](sf::lib::Result res)
    {
        results.push_back(res);
        return true;
    };

    std::string haystack = "myneedledata";
    ASSERT_NO_THROW(m_cache->CompareData(2, haystack, resultCb));

    ASSERT_EQ(1, results.size());

    auto& res = results.front();
    auto nlPart = m_needleData.substr(res.NlOffset, res.MatchLen);
    auto hsPart = haystack.substr(res.HsOffset, res.MatchLen);
    ASSERT_STREQ(nlPart.c_str(), hsPart.c_str());
}