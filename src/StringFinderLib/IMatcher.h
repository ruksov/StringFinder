#pragma once
#include <functional>
#include <memory>
#include <vector>

#include "MatchResult.h"

namespace sf::lib
{
    using ResultHandlerFn = std::function<void(const MatchResult&)>;
    using ResultHandlerFnPtr = std::shared_ptr<ResultHandlerFn>;

    class IMatcher
    {
    public:
        virtual ~IMatcher() = default;

        virtual size_t Match(size_t hsOffset, size_t hsDataIndex, const Data& hsData) = 0;

        void AddResultHandler(ResultHandlerFnPtr fn);
        void RemoveResultHandler(ResultHandlerFnPtr fn);
        bool HasResultHandler(ResultHandlerFnPtr fn);

    protected:
        void NotifyAll(const MatchResult& matchRes);

    private:
        std::vector<ResultHandlerFnPtr> m_handlers;
    };

    using MatcherPtr = std::unique_ptr<IMatcher>;
}