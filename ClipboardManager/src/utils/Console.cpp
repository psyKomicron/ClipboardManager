#include "pch.h"
#include "Console.hpp"

#include <Windows.h>

#include <iostream>

std::atomic_bool clip::utils::Console::consoleInitialized = false;

clip::utils::Console::Console()
{
    if (consoleInitialized) return;

    releaseConsole();

    if (!AllocConsole())
    {
        throw std::runtime_error("Failed to allocate console.");
    }

    resizeConsole(1024);
    redirectConsole();

    MoveWindow(GetConsoleWindow(), 100, 100, 1000, 600, TRUE);

    consoleInitialized = true;
}

clip::utils::Console::~Console()
{
    if (consoleAllocated)
    {
        FreeConsole();
    }
}

void clip::utils::Console::test()
{
    std::wcout << L"Console test" << std::endl;
}

bool clip::utils::Console::initialized()
{
    return consoleInitialized;
}

void clip::utils::Console::redirectConsole()
{
    file_t fp = nullptr;
    
    if (hasInput())
    {
        redirectCon("CONIN$", "r", fp, stdin);
    }

    if (hasOutput())
    {
        redirectCon("CONOUT$", "w", fp, stdout);
    }

    if (hasErr())
    {
        redirectCon("CONOUT$", "w", fp, stderr);
    }

    std::ios::sync_with_stdio();

    std::wcout.clear();
    std::cout.clear();
    std::wcerr.clear();
    std::cerr.clear();
    std::wcin.clear();
    std::cin.clear();
}

void clip::utils::Console::releaseConsole()
{
    file_t fp = nullptr;

    releaseCon(fp, "r", stdin);
    releaseCon(fp, "w", stdout);
    releaseCon(fp, "w", stderr);

    std::ignore = FreeConsole();
}

void clip::utils::Console::resizeConsole(const uint16_t& minLength)
{
    CONSOLE_SCREEN_BUFFER_INFO conInfo{};
    auto outHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (!GetConsoleScreenBufferInfo(outHandle, &conInfo))
    {
        throw std::runtime_error("Failed to resize console, can't open console screen buffer info.");
    }

    if (conInfo.dwSize.Y < minLength)
    {
        conInfo.dwSize.Y = minLength;
    }

    SetConsoleScreenBufferSize(outHandle, conInfo.dwSize);
}

bool clip::utils::Console::hasInput() const
{
    return GetStdHandle(STD_INPUT_HANDLE) != INVALID_HANDLE_VALUE;
}

bool clip::utils::Console::hasOutput() const
{
    return GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE;
}

bool clip::utils::Console::hasErr() const
{
    return GetStdHandle(STD_ERROR_HANDLE) != INVALID_HANDLE_VALUE;
}

void clip::utils::Console::redirectCon(const std::string& con, const std::string& mode, FILE* fp, FILE* io)
{
    if (freopen_s(&fp, con.c_str(), mode.c_str(), io) != 0)
    {
        throw std::runtime_error("Failed to re-allocate io: " + con);
    }
    setvbuf(io, nullptr, _IONBF, 0);
}

void clip::utils::Console::releaseCon(clip::utils::Console::file_t fp, const std::string& mode, clip::utils::Console::file_t io)
{
    freopen_s(&fp, "NUL:", mode.c_str(), io);
    setvbuf(io, nullptr, _IONBF, 0);
}
