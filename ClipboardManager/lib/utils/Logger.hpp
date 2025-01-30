#pragma once
#include <boost/log/sources/logger.hpp>
#include <atomic>
#include <string>

namespace clip::utils
{
    constexpr auto&& useColors = false;

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

        bool isLogBackendInitialized() const;

        void debug(const std::wstring& message);
        void info(const std::wstring& message);
        void error(const std::wstring& message);
        void error(const std::string& message);
        void print(const std::wstring& message, const LogSeverity& severity = LogSeverity::None);

    private:
        static std::atomic_size_t maxClassNameLength;
        static std::atomic_bool boostLoggerInitialized;
        size_t instanceMaxClassNameLength{};
        std::wstring className{};
        boost::log::sources::logger logger{};

        std::wstring formatClassName(const std::wstring_view& className) const;
        void initBoostLogging();
    };
}
