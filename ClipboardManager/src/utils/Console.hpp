#pragma once
#include <atomic>

namespace clip::utils
{
    class Console
    {
    public:
        Console();
        ~Console();

        void test();

        static bool initialized();

    private:
        static std::atomic_bool consoleInitialized;
        bool consoleAllocated = false;

        using file_t = FILE*;

        void redirectConsole();
        void releaseConsole();
        void resizeConsole(const uint16_t& minLength);

        bool hasInput() const;
        bool hasOutput() const;
        bool hasErr() const;
        void redirectCon(const std::string& con, const std::string& mode, FILE* fp, FILE* io);
        void releaseCon(clip::utils::Console::file_t fp, const std::string& mode, clip::utils::Console::file_t io);
    };
}
