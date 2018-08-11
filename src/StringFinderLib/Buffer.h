#pragma once
#include "IReader.h"

namespace sf::lib
{
    using DoubleBuffer = std::array<Data, 2>;

    class Buffer
    {
    public:
        explicit Buffer(IReader& reader);

        bool HasNextData() const noexcept;
        const Data& GetNextData();

    private:
        std::future<bool> LaunchReadThread();

    private:
        IReader & m_reader;
        bool m_hasNext;

        DoubleBuffer m_dBuf;
        Data& m_current;
        Data& m_next;
        std::future<bool> m_waitNext;
    };
}