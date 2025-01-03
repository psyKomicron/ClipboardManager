#include "pch.h"
#include "Logger.hpp"

#include "Console.hpp"
#include "src/utils/helpers.hpp"

#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/trivial.hpp>

#include <iostream>
#include <format>
#include <mutex>

namespace boost::log
{
    using namespace boost::log::sources;
    using namespace boost::log::keywords;
}

namespace clip::utils
{
    std::atomic_size_t Logger::maxClassNameLength = 0;
    
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
        
        BOOST_LOG(logger) << formattedName << clip::utils::to_string(message);

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
        boost::log::add_file_log
        (
            boost::log::file_name = "sample%N.log",
            boost::log::format = "[%TimeStamp%]: %Message%"
        );
        //boost::log::core::get()->set_filter(boost::log::trivial::severity >= 1);
        boost::log::add_common_attributes();
    }
}
