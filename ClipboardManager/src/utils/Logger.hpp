#pragma once
#include <atomic>
#include <string>

namespace clip::utils
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

        void debug(const std::wstring& message) const;
        void info(const std::wstring& message) const;
        void error(const std::wstring& message) const;
        void error(const std::string& message) const;
        void print(const std::wstring& message, const clip::utils::ConsoleColors& color = ConsoleColors::White) const;

    private:
        static const bool useColors = false;
        static std::atomic_size_t maxClassNameLength;
        size_t instanceMaxClassNameLength{};
        std::wstring className{};

        std::wstring formatClassName(const std::wstring& className) const;
    };
}

