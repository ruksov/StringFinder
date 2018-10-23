#include "stdafx.h"
#include "CacheFactory.h"

using namespace testing;
using namespace sf::lib;

namespace
{
    struct TestDiffCache : public Test
    {
    public:
        void CreateCache()
        {
            m_cache = CacheFactory(CacheType::DiffCache, m_data);
        }

        std::string GetDataSubStr(const Result& res)
        {
            return m_data.substr(res.NlOffset, res.MatchLen);
        }

        std::string GetCmpDataSubStr(const Result& res)
        {
            return m_cmpData.substr(res.HsDataOffset, res.MatchLen);
        }

        bool CompareData(const Result& res)
        {
            return GetDataSubStr(res) == GetCmpDataSubStr(res);
        }


    protected:
        Data m_data;
        Data m_cmpData;
        size_t m_cmpOffset = 0;
        CachePtr m_cache;
    };
}

TEST_F(TestDiffCache, EmptyCache)
{
    ASSERT_NO_THROW(CreateCache());
    ASSERT_TRUE(m_cache->GetCacheData().empty());
}

TEST_F(TestDiffCache, GetCacheData)
{
    m_data = "some_data";
    ASSERT_NO_THROW(CreateCache());
    ASSERT_STREQ(m_data.data(), m_cache->GetCacheData().data());
}

TEST_F(TestDiffCache, Reset)
{
    m_data = "some_data";
    ASSERT_NO_THROW(CreateCache());
    ASSERT_STREQ(m_data.data(), m_cache->GetCacheData().data());

    m_data = "another_data";
    ASSERT_NO_THROW(m_cache->Reset(m_data));
    ASSERT_STREQ(m_data.data(), m_cache->GetCacheData().data());
}

TEST_F(TestDiffCache, GetFirstResult_OutOfRange)
{
    m_data = "some_data";
    ASSERT_NO_THROW(CreateCache());

    m_cmpOffset = 1;
    ASSERT_GT(m_cmpOffset, m_cmpData.size());
    ASSERT_ANY_THROW(m_cache->GetFirstResult(m_cmpOffset, m_cmpData));
}

TEST_F(TestDiffCache, GetFirstResult_WithOneMatch)
{
    m_data = "some_data";
    ASSERT_NO_THROW(CreateCache());

    m_cmpOffset = 0;
    m_cmpData = "some";
    auto res = m_cache->GetFirstResult(m_cmpOffset, m_cmpData);
    
    ASSERT_TRUE(res);
    ASSERT_STREQ("some", GetDataSubStr(res.value()).data());
}

TEST_F(TestDiffCache, GetFirstResult_WithTwoMatches)
{
    m_data = "some_data_some";
    ASSERT_NO_THROW(CreateCache());

    m_cmpOffset = 0;
    m_cmpData = "some";
    auto res = m_cache->GetFirstResult(m_cmpOffset, m_cmpData);

    ASSERT_TRUE(res);
    ASSERT_EQ(0, res->HsDataOffset);
    ASSERT_STREQ("some", GetDataSubStr(res.value()).data());

    // try repeat call
    res = m_cache->GetFirstResult(m_cmpOffset, m_cmpData);

    ASSERT_TRUE(res);
    ASSERT_EQ(0, res->HsDataOffset);
    ASSERT_STREQ("some", GetDataSubStr(res.value()).data());
}

TEST_F(TestDiffCache, GetNextResult_OutOfRange)
{
    m_data = "some_data";
    ASSERT_NO_THROW(CreateCache());

    // wrong cache offset
    Result prevRes;
    prevRes.NlOffset = 42;
    ASSERT_GT(prevRes.NlOffset, m_data.size());
    ASSERT_ANY_THROW(m_cache->GetNextResult(prevRes, m_data));

    // wrong compare data offset
    prevRes = Result();
    prevRes.HsDataOffset = 42;
    ASSERT_GT(prevRes.HsDataOffset, m_cmpData.size());
    auto res = m_cache->GetNextResult(prevRes, m_data);
    ASSERT_FALSE(res);

    // wrong match length
    prevRes = Result();
    prevRes.MatchLen = 42;
    ASSERT_GT(prevRes.HsDataOffset + prevRes.MatchLen, m_cmpData.size());
    res = m_cache->GetNextResult(prevRes, m_data);
    ASSERT_FALSE(res);
}

TEST_F(TestDiffCache, GetNextResult_FalseNextMatch)
{
    m_data = "some_data_some";
    ASSERT_NO_THROW(CreateCache());

    // get first match result
    m_cmpData = "datadata";
    auto res = m_cache->GetFirstResult(m_cmpOffset, m_cmpData);
    ASSERT_TRUE(res);
    ASSERT_TRUE(CompareData(res.value()));

    // must return false result
    ASSERT_FALSE(m_cache->GetNextResult(res.value(), m_cmpData));
}

TEST_F(TestDiffCache, GetNextResult_HasNextMatch)
{
    m_data = "some_data_datadata";
    ASSERT_NO_THROW(CreateCache());

    // get first match result
    m_cmpData = "datadata";
    auto res = m_cache->GetFirstResult(m_cmpOffset, m_cmpData);
    ASSERT_TRUE(res);
    ASSERT_TRUE(CompareData(res.value()));

    // get next match result
    auto nextRes = m_cache->GetNextResult(res.value(), m_cmpData);
    ASSERT_TRUE(nextRes);
    ASSERT_GT(nextRes->MatchLen, res->MatchLen);
    ASSERT_EQ(m_cmpData.size(), nextRes->MatchLen);

    ASSERT_TRUE(CompareData(nextRes.value()));
    ASSERT_STREQ("datadata", GetDataSubStr(nextRes.value()).data());

    // must return false result
    ASSERT_FALSE(m_cache->GetNextResult(nextRes.value(), m_cmpData));
}