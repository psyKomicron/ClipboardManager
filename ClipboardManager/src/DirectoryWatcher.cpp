#include "pch.h"
#include "DirectoryWatcher.hpp"

#include <Windows.h>

void clip::DirectoryWatcher::startWatching()
{
    std::atomic_flag waitFlag{};
    int retValue = 0;
    backgroundThread = std::jthread(&DirectoryWatcher::watchDirectoryChanges, this, &waitFlag, &retValue);

    waitFlag.wait(false);
    waitFlag.clear();
}

void clip::DirectoryWatcher::watchDirectoryChanges(std::atomic_flag* waitFlag, int* retValue)
{
    watcherRunning = true;

    auto handle = FindFirstChangeNotificationW(path.wstring().c_str(), false, FILE_NOTIFY_CHANGE_FILE_NAME);
    if (handle == INVALID_HANDLE_VALUE)
    {
        *retValue = -1;
    }

    if (handle == nullptr)
    {
        *retValue = -2;
    }

    waitFlag->test_and_set();
    waitFlag->notify_one();

    while (runThread)
    {
        logger.info(L"Waiting for directory changes (path: '" + path.wstring() + L"')");

        auto status = WaitForSingleObjectEx(handle, INFINITE, true);
    }
}
