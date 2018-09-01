#pragma once
#include <string>

namespace sf::lib
{
    using Data = std::string;

    struct IReader
    {
        IReader() = default;
        virtual ~IReader() = default;

        IReader(const IReader&) = delete;
        IReader& operator=(const IReader&) = delete;

        IReader(IReader&&) = delete;
        IReader& operator=(IReader&&) = delete;

        virtual void Reset() = 0;
        virtual void ReadNext(Data& data) = 0;
        virtual bool IsEnd() const noexcept = 0;
        virtual size_t GetIndex() const noexcept = 0;
    };
}