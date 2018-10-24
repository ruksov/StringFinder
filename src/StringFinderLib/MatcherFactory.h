#pragma once
#include "IMatcher.h"
#include "Exceptions.h"
#include "ThreasholdMatcher.h"

namespace sf
{
    enum class MatcherType
    {
        ThreasholdMatcher = 0,
        Unknown
    };

    using MatcherPtr = lib::MatcherPtr;

    template<typename ...CtorArgs>
    MatcherPtr MatcherFactory(MatcherType matcherType, CtorArgs&&... ctorArgs)
    {
        MatcherPtr matcher;

        switch (matcherType)
        {
        case MatcherType::ThreasholdMatcher:
            matcher = std::make_unique<lib::ThreasholdMatcher>(std::forward<CtorArgs>(ctorArgs)...);
            break;

        case MatcherType::Unknown:
        default:
            THROW("Failed to create matcher. Unknown matcher type.")
                break;
        }

        return matcher;
    }
}