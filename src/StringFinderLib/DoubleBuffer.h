#pragma once
#include "IReader.h"

namespace sf::lib
{
    using ReaderPtr = std::unique_ptr<IReader>;

    class DoubleBuffer
    {
    public:
        explicit DoubleBuffer(ReaderPtr reader);

        void Reset();

        const std::reference_wrapper<Data> GetNext();
        size_t GetIndex() const noexcept;
        size_t GetDataCount() const noexcept;

    private:
        std::future<void> LaunchReadThread();

    private:
        ReaderPtr m_reader;
        
        size_t m_index = 0;
        std::array<Data, 2> m_buffer;
        Data& m_current = m_buffer.at(0);
        Data& m_next = m_buffer.at(1);
        std::future<void> m_waitNext;
    };
}