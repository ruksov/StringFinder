#pragma once
#include <functional>

#include "MatchResult.h"

namespace sf::lib
{
    using ResultHandlerFn = std::function<void(const MatchResult&)>;
    using ResultHandlerFnPtr = std::shared_ptr<ResultHandlerFn>;

    class IMatcher
    {
    public:
        virtual ~IMatcher() = default;

        virtual void Match(size_t hsOffset, const Data& hsData) = 0;

        void AddResultHandler(ResultHandlerFnPtr fn);
        void RemoveResultHandler(ResultHandlerFnPtr fn);
        bool HasResultHandler(ResultHandlerFnPtr fn);

        void NotifyAll(const MatchResult& matchRes);

    private:
        std::vector<ResultHandlerFnPtr> m_handlers;
    };
}