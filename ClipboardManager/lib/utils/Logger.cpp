#include "pch.h"
#include "Logger.hpp"

#include "Console.hpp"
#include "lib/utils/helpers.hpp"
#include "lib/Settings.hpp"

#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/trivial.hpp>

#include <winrt/Windows.Storage.h>

#include <iostream>
#include <format>
#include <mutex>
#include <filesystem>

namespace boost::log
{
    using namespace boost::log::sources;
    using namespace boost::log::keywords;
}

namespace clip::utils
{
    constexpr bool USE_LOG_FILE = false;

    std::atomic_size_t Logger::maxClassNameLength = 0;
    std::atomic_bool Logger::boostLoggerInitialized = false;
}

namespace clip::utils
{   
    Logger::Logger(const std::wstring& className) :
        className{ className }
    {
        if constexpr (USE_LOG_FILE)
        {
            static std::once_flag flag{};
            std::call_once(flag, [this]()
            {
                try
                {
                    initBoostLogging();
                }
                catch (std::exception& except)
                {
                    error("Impossible to initialize boost logging: " + std::string(except.what()));
                }
            });
        }
    }

    bool Logger::isLogBackendInitialized() const
    {
        return boostLoggerInitialized;
    }

    void Logger::debug(const std::wstring& message)
    {
#ifdef _DEBUG
        print(message, LogSeverity::Debug);
#endif // _DEBUG
    }

    void Logger::info(const std::wstring& message)
    {
        print(message, LogSeverity::Info);
    }

    void Logger::error(const std::wstring& message)
    {
        print(message, LogSeverity::Error);
    }

    void Logger::error(const std::string& message)
    {
        print(to_wstring(message), LogSeverity::Error);
    }

    void Logger::print(const std::wstring& message, const LogSeverity& severity)
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
                std::wcout << std::format(L"{}  DEBUG:  {}\n", formattedName, message);
                break;
            }

            case LogSeverity::Info:
            {
                std::wcout << std::format(L"{}  INFO:   {}\n", formattedName, message);
                break;
            }

            case LogSeverity::Error:
            {
                std::wcout << std::format(L"{}  ERROR:  {}\n", formattedName, message);
                break;
            }

            case LogSeverity::None:
            default:
            {
                std::wcout << std::format(L"{}  LOG:    {}\n", formattedName, message);
                break;
            }
        }

        if constexpr (USE_LOG_FILE)
        {
            BOOST_LOG(logger) << formattedName << L' ' << clip::utils::to_string(message);
        }
    }


    std::wstring Logger::formatClassName(const std::wstring_view& className) const
    {
        return std::format(
            L"[{}]{}",
            className, std::wstring(maxClassNameLength - className.size(), L' ')
        );
    }

    void Logger::initBoostLogging()
    {
        clip::Settings settings{};
        if (settings.get<bool>(L"LoggingEnabled").value_or(false))
        {
            std::filesystem::path documentsLibPath{ std::wstring(winrt::Windows::Storage::KnownFolders::DocumentsLibrary().Path()) };
            auto logFilePath = documentsLibPath / L"cm_log%N.log";

            auto userLogFilePath = settings.get<std::filesystem::path>(L"LogFilePath");
            if (userLogFilePath)
            {
                logFilePath = userLogFilePath.value() / logFilePath;
            }

            auto&& fileLog = boost::log::add_file_log
            (
                boost::log::file_name = logFilePath.string(),
                boost::log::format = "[%TimeStamp%]: %Message%",
                boost::log::auto_flush = true
            );

            boost::log::add_common_attributes();

            boostLoggerInitialized = (fileLog != nullptr);
        }
    }
}
