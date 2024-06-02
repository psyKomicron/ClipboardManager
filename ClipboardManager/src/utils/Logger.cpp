#include "pch.h"
#include "Logger.hpp"

#include "Console.hpp"

#include <iostream>
#include <format>

std::atomic_size_t clipmgr::utils::Logger::maxClassNameLength = 10;

clipmgr::utils::Logger::Logger(const std::wstring& className) :
    className{ className }
{
    if (maxClassNameLength < className.size())
    {
        maxClassNameLength = className.size();
    }

    if (!Console::initialized())
    {
        static Console console{};
    }
}

void clipmgr::utils::Logger::debug(const std::wstring& message) const
{
#ifdef _DEBUG
    print(message, ConsoleColors::Yellow);
#endif // _DEBUG
}

void clipmgr::utils::Logger::info(const std::wstring& message) const
{
    print(message, ConsoleColors::Green);
}

void clipmgr::utils::Logger::error(const std::wstring& message) const
{
    print(message, ConsoleColors::Red);
}

void clipmgr::utils::Logger::print(const std::wstring& message, const clipmgr::utils::ConsoleColors& color) const
{
    const wchar_t* colorEnd = L"\033[0m";

    auto&& formattedName = formatClassName(className);

    switch (color)
    {
        case ConsoleColors::Yellow:
        {
            if constexpr (useColors)
            {
                const wchar_t* colorBegin = L"\x1B[33m";
                std::wcout << colorBegin << L"[DEBUG]   " << formattedName << L"   " << message << colorEnd << std::endl;
            }
            else
            {
                std::wcout << L"[DEBUG]   " << formattedName << L"   " << message << std::endl;
            }
            break;
        }

        case ConsoleColors::Green:
        {
            if constexpr (useColors)
            {
                const wchar_t* colorBegin = L"\x1B[32m";
                std::wcout << colorBegin << L" [INFO]    " << formattedName << L"   " << message << colorEnd << std::endl;
            }
            else
            {
                std::wcout << L" [INFO]   " << formattedName << L"   " << message << std::endl;
            }
            break;
        }

        case ConsoleColors::Red:
        {
            if constexpr (useColors)
            {
                const wchar_t* colorBegin = L"\x1B[31m";
                std::wcout << colorBegin << L"[ERROR]   " << formattedName << L"   " << message << colorEnd << std::endl;
            }
            else
            {
                std::wcout << L"[ERROR]   " << formattedName << L"   " << message << std::endl;
            }
            break;
        }

        case ConsoleColors::White:
        default:
        {
            std::wcout << L"[LOG]     " << formattedName << L"   " << message << std::endl;
            break;
        }
    }
}

std::wstring clipmgr::utils::Logger::formatClassName(const std::wstring& className) const
{
    const std::wstring padding = std::wstring((maxClassNameLength - className.size()) / 2, L' ');
    auto&& formattedName = std::vformat(L"{}{}{}", std::make_wformat_args(
        padding,
        className,
        padding
    ));

    return formattedName;
}
