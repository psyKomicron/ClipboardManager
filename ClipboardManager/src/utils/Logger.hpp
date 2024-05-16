#pragma once
#include <atomic>
#include <string>

namespace clipmgr::utils
{
    enum class ConsoleColors
    {
        Yellow,
        Green,
        White,
        Red
    };

    class Logger
    {
    public:
        Logger(const std::wstring& className);

        void debug(const std::wstring& message);
        void info(const std::wstring& message);
        void error(const std::wstring& message);
        void print(const std::wstring& message, const clipmgr::utils::ConsoleColors& color = ConsoleColors::White);

    private:
        static const bool useColors = false;
        static std::atomic_size_t maxClassNameLength;
        size_t instanceMaxClassNameLength{};
        std::wstring className{};

        std::wstring formatClassName(const std::wstring& className);
    };
}

