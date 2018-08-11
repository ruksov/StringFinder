#include "stdafx.h"
#include "Buffer.h"

using namespace testing;

namespace
{
    class MockReader : public sf::lib::IReader, public Mock
    {
        MOCK_METHOD0(MockReset, void());
        MOCK_METHOD0(MockHasNext, bool());
        MOCK_METHOD1(MockReadNext, bool(sf::lib::Data&));
    };

    class TestBuffer : public Test
    {
    protected:
        void SetUp() override
        {

        }

        void TearDown() override
        {

        }

    protected:
        std::string m_srcBuf;
        std::unique_ptr<sf::lib::Buffer> m_testBuf;
    };
}