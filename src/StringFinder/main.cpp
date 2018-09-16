// StringFinder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "StringFinder.h"
#include "Exceptions.h"
#include "Log.h"
#include "DiffCache.h"

int main()
{
    try
    {
        size_t threshold = 0;
        std::wstring needlePath;
        std::wstring haystackPath;
        std::wcout << "Input threshold -> ";
        std::wcin >> threshold;
        std::wcin.get();
        std::wcout << "Input needle file path -> ";
        std::getline(std::wcin, needlePath);
        std::wcout << "input haystack file path -> ";
        std::getline(std::wcin, haystackPath);

        sf::StringFinder finder;
        finder.Run(threshold, std::move(needlePath), std::move(haystackPath));
    }
    catch (std::exception& ex)
    {
        LOG_ERROR(ex.what());
        return -1;
    }

    return 0;
}