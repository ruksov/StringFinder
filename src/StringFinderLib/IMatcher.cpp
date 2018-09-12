#include "stdafx.h"
#include "IMatcher.h"
#include "Exceptions.h"

namespace sf::lib
{
    void IMatcher::AddResultHandler(ResultHandlerFnPtr fn)
    {
        THROW_IF(HasResultHandler(fn), "Faile to add result handler. Such handler already exist.");
        m_handlers.push_back(std::move(fn));
    }

    void IMatcher::RemoveResultHandler(ResultHandlerFnPtr fn)
    {
        auto removeIt = std::remove(m_handlers.begin(), m_handlers.end(), fn);
        assert(std::distance(removeIt, m_handlers.end()) == 1);
        m_handlers.erase(removeIt, m_handlers.end());
    }

    bool IMatcher::HasResultHandler(ResultHandlerFnPtr fn)
    {
        return std::find(m_handlers.begin(), m_handlers.end(), fn) != m_handlers.end();
    }

    void IMatcher::NotifyAll(const MatchResult & matchRes)
    {
        try
        {
            for (auto& handler : m_handlers)
            {
                handler->operator()(matchRes);
            }
        }
        catch (std::exception& e)
        {
            THROW("Failed to notify all. Some handler raise an exception: " << e.what());
        }
    }
}