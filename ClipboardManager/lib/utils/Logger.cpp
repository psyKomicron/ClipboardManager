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
    constexpr bool USE_LOG_FILE = true;

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
                initBoostLogging();
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
        
        if constexpr (USE_LOG_FILE)
        {
            BOOST_LOG(logger) << formattedName << clip::utils::to_string(message);
        }

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
            {
                std::wcout << formattedName << L"  LOG:    " << message << std::endl;
                break;
            }
        }

    }


    std::wstring Logger::formatClassName(const std::wstring& className) const
    {
        const std::wstring padding = std::wstring(maxClassNameLength - className.size(), L' ');

        auto&& formattedName = std::vformat(L"[{}]{}", std::make_wformat_args(
            className,
            padding
        ));

        return formattedName;
    }

    void Logger::initBoostLogging()
    {
        std::filesystem::path logFileName{ "cm_log%N.log" };
        auto logFilePath = clip::Settings().get<std::filesystem::path>(L"LogFilePath");
        if (logFilePath)
        {
            logFileName = logFilePath.value() / logFileName;
        }

        auto&& fileLog = boost::log::add_file_log
        (
            boost::log::file_name = logFileName.string(),
            boost::log::format = "[%TimeStamp%]: %Message%",
            boost::log::auto_flush = true
        );

        boost::log::add_common_attributes();

        boostLoggerInitialized = (fileLog != nullptr);
    }
}
