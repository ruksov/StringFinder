#include "stdafx.h"
#include "DoubleBuffer.h"

namespace sf::lib
{
    DoubleBuffer::DoubleBuffer(ReaderPtr reader)
        : m_reader(std::move(reader))
        , m_waitNext(!m_reader->IsEnd() ? LaunchReadThread() : std::future<void>())
    {
    }

    void DoubleBuffer::Reset()
    {
        if (m_waitNext.valid())
        {
            m_waitNext.get();
        }

        m_reader->Reset();
        m_index = 0;
        m_current.clear();
        m_next.clear();
        m_waitNext = !m_reader->IsEnd() ? LaunchReadThread() : std::future<void>();
    }

    const std::reference_wrapper<Data> DoubleBuffer::GetNext()
    {
        if (m_waitNext.valid())
        {
            m_waitNext.get();
        }

        // Increment index when we change data in current buffer
        if (!m_current.empty())
        {
            ++m_index;
        }

        m_current.swap(m_next);

        if(!m_reader->IsEnd())
        {
            m_waitNext = LaunchReadThread();
        }
        else if(!m_next.empty())
        {
            m_next.clear();
        }

        return m_current;
    }

    size_t DoubleBuffer::GetIndex() const noexcept
    {
        return m_index;
    }

    size_t DoubleBuffer::GetDataCount() const noexcept
    {
        return m_reader->GetDataCount();
    }

    std::future<void> DoubleBuffer::LaunchReadThread()
    {
        return std::async(std::launch::async,
            [&rd = m_reader, &next = m_next]() { rd->ReadNext(next); });
    }
}