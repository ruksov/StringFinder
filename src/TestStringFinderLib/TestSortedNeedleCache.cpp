#include "stdafx.h"
#include "SortedNeedleCache.h"

using namespace testing;

namespace
{
    struct TestSortedNeedleCache : Test
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
            m_cache = std::make_unique<sf::lib::SortedNeedleCache>(m_path);
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
        std::unique_ptr<sf::lib::INeedleCache> m_cache;
    };
}

TEST_F(TestSortedNeedleCache, GetNeedle)
{
    m_needleData = "test Needle";
    FillFile();
    ASSERT_NO_THROW(ResetNeedleCache());

    auto& ndl = m_cache->GetNeedle();
    ASSERT_STREQ(m_needleData.c_str(), ndl.c_str());
}

TEST_F(TestSortedNeedleCache, GetIndexList_EmptyNeedle)
{
    FillFile();
    ASSERT_NO_THROW(ResetNeedleCache());

    const auto charMin = std::numeric_limits<char>::min();
    char c = std::numeric_limits<char>::min();

    do
    {
        auto& indxList = m_cache->GetOffsetList(c);
        ASSERT_EQ(0, indxList.size());
        ++c;
    } while (c != charMin);
}

TEST_F(TestSortedNeedleCache, GetIndexList_Normal)
{
    m_needleData = "testtest";
    FillFile();
    ASSERT_NO_THROW(ResetNeedleCache());

    // get index list for all needle character
    auto& tIndxList = m_cache->GetOffsetList('t');
    auto& eIndxList = m_cache->GetOffsetList('e');
    auto& sIndxList = m_cache->GetOffsetList('s');

    auto checkList = [&ndlData = m_needleData](char c, size_t charCount, auto& indxList)
    {
        ASSERT_EQ(charCount, indxList.size());
        ASSERT_TRUE(std::is_sorted(indxList.begin(), indxList.end()));
        for (auto i : indxList)
        {
            ASSERT_EQ(c, ndlData[i]);
        }
    };

    checkList('t', 4, tIndxList);
    checkList('e', 2, eIndxList);
    checkList('s', 2, sIndxList);

    const auto charMin = std::numeric_limits<char>::min();
    char c = std::numeric_limits<char>::min();

    do
    {
        if (c != 't' && c != 'e' && c != 's')
        {
            // index lists for all other chars must be empty
            auto& indxList = m_cache->GetOffsetList(c);
            ASSERT_EQ(0, indxList.size());
        }
        ++c;
    } while (c != charMin);
}