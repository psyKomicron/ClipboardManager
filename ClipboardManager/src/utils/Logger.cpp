#include "pch.h"
#include "Logger.hpp"

#include "Console.hpp"
#include "src/utils/helpers.hpp"

#include <iostream>
#include <format>

std::atomic_size_t clip::utils::Logger::maxClassNameLength = 0;

clip::utils::Logger::Logger(const std::wstring& className) :
    className{ className }
{
}

void clip::utils::Logger::debug(const std::wstring& message) const
{
#ifdef _DEBUG
    print(message, LogSeverity::Debug);
#endif // _DEBUG
}

void clip::utils::Logger::info(const std::wstring& message) const
{
    print(message, LogSeverity::Info);
}

void clip::utils::Logger::error(const std::wstring& message) const
{
    print(message, LogSeverity::Error);
}

void clip::utils::Logger::error(const std::string& message) const
{
    print(clip::utils::convert(message), LogSeverity::Error);
}

void clip::utils::Logger::print(const std::wstring& message, const LogSeverity& severity) const
{
    if (maxClassNameLength < className.size())
    {
        maxClassNameLength = className.size();
    }

    auto&& formattedName = formatClassName(className);

    switch (severity)
    {
        case LogSeverity::Debug:
        {
            std::wcout << formattedName << L"  DEBUG:  " << message << std::endl;
            break;
        }

        case LogSeverity::Info:
        {
            std::wcout << formattedName << L"  INFO:   " << message << std::endl;
            break;
        }

        case LogSeverity::Error:
        {
            std::wcout << formattedName << L"  ERROR:  " << message << std::endl;
            break;
        }

        case LogSeverity::None:
        default:
            std::wcout << formattedName << L"  LOG:    " << message << std::endl;
            break;
    }
}

std::wstring clip::utils::Logger::formatClassName(const std::wstring& className) const
{
    const std::wstring padding = std::wstring(maxClassNameLength - className.size(), L' ');

    auto&& formattedName = std::vformat(L"[{}]{}", std::make_wformat_args(
        className,
        padding
    ));

    return formattedName;
}
