// StringFinder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "StringFinder.h"
#include "Exceptions.h"
#include "Log.h"
#include "DiffCache.h"

struct ArgsInfo
{
    size_t Threshold = 0;
    std::string NeedlePath;
    std::string HaystackPath;
#pragma warning (suppress : 26495)  // always initialize Threshold (bug)
};

std::string GetHelpStr()
{
    return "Usage:\n needle_file haystack_file threshold";
}

ArgsInfo ParseArgs(int argc, const char* argv[])
{
    THROW_IF(argc < 4, "Too less arguments");

    ArgsInfo argsInfo;

    argsInfo.NeedlePath = argv[1];
    argsInfo.HaystackPath = argv[2];
    argsInfo.Threshold = static_cast<size_t>(atoi(argv[3]));

    return argsInfo;
}

int main(int argc, const char* argv[])
{
    sf::log::InitLog();
    ArgsInfo argsInfo;

    try
    {
        argsInfo = ParseArgs(argc, argv);
        LOG_INFO("Args: " << argsInfo.HaystackPath << '\n'
            << argsInfo.NeedlePath << '\n'
            << argsInfo.Threshold);
    }
    catch (std::exception& ex)
    {
        LOG_ERROR(ex.what() << '\n' << GetHelpStr());
        return 1;
    }

    try
    {
        sf::StringFinder finder;
        finder.Run(argsInfo.Threshold, std::move(argsInfo.NeedlePath), std::move(argsInfo.HaystackPath));
    }
    catch (std::exception& ex)
    {
        LOG_ERROR(ex.what());
        return -1;
    }

    return 0;
}