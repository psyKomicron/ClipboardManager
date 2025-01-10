#include "pch.h"
#include "Process.hpp"

clip::Process::Process(const std::wstring& command) :
    command{ command }
{
}

clip::Process::~Process()
{
    if (processHandle)
    {
        CloseHandle(processHandle);
    }

    if (processThreadHandle)
    {
        CloseHandle(processThreadHandle);
    }
}

void clip::Process::invoke()
{
    STARTUPINFOW info{ sizeof(STARTUPINFOW) };
    PROCESS_INFORMATION processInfo{};
    if (CreateProcessW(nullptr, command.data(), nullptr, nullptr, false, DETACHED_PROCESS, nullptr, nullptr, &info, &processInfo))
    {
        processHandle = processInfo.hProcess;
        processThreadHandle = processInfo.hThread;

        if (CloseHandle(processInfo.hProcess))
        {
            processHandle = nullptr;
        }

        if (CloseHandle(processInfo.hThread))
        { 
            processThreadHandle = nullptr;
        }
    }
    else
    {
        winrt::throw_last_error();
    }
}
