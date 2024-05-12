#pragma once
namespace clipmgr::utils
{
    class Console
    {
    public:
        Console();
        ~Console();

        void test();

    private:
        using file_t = FILE*;
        bool consoleAllocated = false;

        void redirectConsole();
        void releaseConsole();
        void resizeConsole(const uint16_t& minLength);

        bool hasInput() const;
        bool hasOutput() const;
        bool hasErr() const;
        void redirectCon(const std::string& con, const std::string& mode, FILE* fp, FILE* io);
        void releaseCon(clipmgr::utils::Console::file_t fp, const std::string& mode, clipmgr::utils::Console::file_t io);
    };
}
