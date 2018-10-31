#pragma once
#include "IMatcher.h"
#include "Exceptions.h"
#include "ThresholdMatcher.h"

namespace sf::lib
{
    enum class MatcherType
    {
        ThresholdMatcher = 0,
        Unknown
    };

    template<typename ...CtorArgs>
    MatcherPtr MatcherFactory(MatcherType matcherType, CtorArgs&&... ctorArgs)
    {
        MatcherPtr matcher;

        switch (matcherType)
        {
        case MatcherType::ThresholdMatcher:
            matcher = std::make_unique<ThresholdMatcher>(std::forward<CtorArgs>(ctorArgs)...);
            break;

        case MatcherType::Unknown:
        default:
            THROW("Failed to create matcher. Unknown matcher type.")
                break;
        }

        return matcher;
    }
}