#pragma once

namespace sf::lib
{
    using OnProgressCb = std::function<void(size_t current, size_t end)>;

    class ProgressBarCollection
    {
    public:
        explicit ProgressBarCollection(size_t end);

        bool AddCallback(std::string cbName, OnProgressCb cb);
        void RemoveCallback(std::string cbName);
        void Clear() noexcept;

        void OnProgressChange(size_t current);

    private:
        const size_t m_end;
        std::map<std::string, OnProgressCb> m_callbacks;
    };
}