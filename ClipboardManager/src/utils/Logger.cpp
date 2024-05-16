#include "pch.h"
#include "Logger.hpp"

#include "Console.hpp"

#include <iostream>

std::atomic_size_t clipmgr::utils::Logger::maxClassNameLength = 10;

clipmgr::utils::Logger::Logger(const std::wstring& className) :
    className{ formatClassName(className) }
{
    if (!Console::initialized())
    {
        static Console console{};
    }
}

void clipmgr::utils::Logger::debug(const std::wstring& message)
{
    print(message, ConsoleColors::Yellow);
}

void clipmgr::utils::Logger::info(const std::wstring& message)
{
    print(message, ConsoleColors::Green);
}

void clipmgr::utils::Logger::error(const std::wstring& message)
{
    print(message, ConsoleColors::Red);
}

void clipmgr::utils::Logger::print(const std::wstring& message, const clipmgr::utils::ConsoleColors& color)
{
    const wchar_t* colorEnd = L"\033[0m";

    /*if (instanceMaxClassNameLength != maxClassNameLength)
    {
        className = formatClassName(className);
    }*/

    switch (color)
    {
        case ConsoleColors::Yellow:
        {
            if constexpr (useColors)
            {
                const wchar_t* colorBegin = L"\x1B[33m";
                std::wcout << colorBegin << L"[DEBUG]   " << className << L"   " << message << colorEnd << std::endl;
            }
            else
            {
                std::wcout << L"[DEBUG]   " << className << L"   " << message << std::endl;
            }
            break;
        }
        
        case ConsoleColors::Green:
        {
            if constexpr (useColors)
            {
                const wchar_t* colorBegin = L"\x1B[32m";
                std::wcout << colorBegin << L"[INFO]    " << className << L"   " << message << colorEnd << std::endl;
            }
            else
            {
                std::wcout << L"[INFO]    " << className << L"   " << message << std::endl;
            }
            break;
        }
        
        case ConsoleColors::Red:
        {
            if constexpr (useColors)
            {
                const wchar_t* colorBegin = L"\x1B[31m";
                std::wcout << colorBegin << L"[ERROR]   " << className << L"   " << message << colorEnd << std::endl;
            }
            else
            {
                std::wcout << L"[ERROR]   " << className << L"   " << message << std::endl;
            }
            break;
        }

        case ConsoleColors::White:
        default:
        {
            std::wcout << L"[LOG]     " << className << L"   " << message << std::endl;
            break;
        }
    }
}

std::wstring clipmgr::utils::Logger::formatClassName(const std::wstring& className)
{
    if (maxClassNameLength < className.size())
    {
        maxClassNameLength = className.size();
    }

    const size_t padding = maxClassNameLength - className.size();

    auto&& formattedName = std::vformat(L"[{}{}{}]", std::make_wformat_args(
        std::wstring(padding / 2, L' '),
        className,
        std::wstring(padding / 2, L' ')
    ));

    return formattedName;
}
