#pragma once
#include <boost/log/sources/logger.hpp>
#include <atomic>
#include <string>

namespace clip::utils
{
    constexpr bool USE_LOG_FILE = true;

    enum class LogSeverity
    {
        Debug,
        Info,
        Error,
        None
    };

    class Logger
    {
    public:
        Logger(const std::wstring& className);

        void debug(const std::wstring& message);
        void info(const std::wstring& message);
        void error(const std::wstring& message);
        void error(const std::string& message);
        void print(const std::wstring& message, const LogSeverity& severity = LogSeverity::None);

    private:
        static const bool useColors = false;
        static std::atomic_size_t maxClassNameLength;
        size_t instanceMaxClassNameLength{};
        std::wstring className{};
#if 1
        boost::log::sources::logger logger{};
#endif

        std::wstring formatClassName(const std::wstring& className) const;
        void initBoostLogging();
    };
}

