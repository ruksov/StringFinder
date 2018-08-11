#include "stdafx.h"
#include "Buffer.h"

namespace sf::lib
{
    Buffer::Buffer(IReader & reader)
        : m_reader(reader)
        , m_hasNext(m_reader.HasNext())
        , m_current(m_dBuf.at(0))
        , m_next(m_dBuf.at(1))
        , m_waitNext(LaunchReadThread())
    {
    }

    bool Buffer::HasNextData() const noexcept
    {
        return m_hasNext;
    }

    const Data& Buffer::GetNextData()
    {
        if (m_waitNext.get())
        {
            m_current.swap(m_next);
            m_hasNext = m_reader.HasNext();
            m_waitNext = LaunchReadThread();
        }
        return m_current;
    }

    std::future<bool> Buffer::LaunchReadThread()
    {
        return std::async(std::launch::async,
            [&rd = m_reader, &next = m_next]() { return rd.ReadNext(next); });
    }
}
