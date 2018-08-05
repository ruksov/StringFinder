#pragma once

namespace sf::lib
{
    using Data = std::vector<char>;

    struct IReader
    {
        IReader() = default;
        virtual ~IReader() = default;

        IReader(const IReader&) = delete;
        IReader& operator=(const IReader&) = delete;

        IReader(IReader&&) = delete;
        IReader& operator=(IReader&&) = delete;

        virtual void ResetRead() = 0;
        virtual bool HasNext() const noexcept = 0;
        virtual bool ReadNext(Data& data) = 0;
    };
}