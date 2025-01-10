#include "pch.h"
#include "FileWatcher.hpp"

#include <Windows.h>

#include <functional>
#include <fstream>

namespace clip
{
    FileWatcher::FileWatcher(const std::function<void()>& event) :
        eventCallback{ event }
    {
    }

    std::filesystem::path FileWatcher::path() const
    {
        return _path;
    }

    void FileWatcher::startWatching(const std::filesystem::path& path)
    {
        _path = path;

        if (!std::filesystem::exists(_path))
        {
            throw std::invalid_argument("path");
        }

        std::ifstream stream{ _path, std::ios::binary };
        if (!stream.is_open())
        {
            throw std::runtime_error("Opening stream");   
        }

        lastHash = hasher.hash(stream);

        std::atomic_flag waitFlag{};
        int retValue = 0;
        backgroundThread = std::jthread(&FileWatcher::watchDirectoryChanges, this, &waitFlag, &retValue);

        waitFlag.wait(false);
        waitFlag.clear();

        if (retValue == 2)
        {
            throw std::wstring(L"FindFirstChangeNotificationW returned INVALID_HANDLE_VALUE. File/directory wasn't found.");
        }
        else if (retValue != 0)
        {
            throw std::wstring(L"FindFirstChangeNotificationW returned INVALID_HANDLE_VALUE. GetLastError -> " + std::to_wstring(retValue));
        }
    }

    void FileWatcher::findChanges()
    {
        std::ifstream stream{ _path, std::ios::binary };
        if (stream.is_open())
        {
            auto hash = hasher.hash(stream);

            if (hash != lastHash)
            {
                logger.info(L"File changed !");

                eventCallback();
            }
            else
            {
                logger.debug(L"File is the same (same hash).");
            }

            lastHash = hash;
        }
    }

    void FileWatcher::watchDirectoryChanges(std::atomic_flag* waitFlag, int* retValue)
    {
        watcherRunning = true;

        auto pathString = _path.parent_path() / L"";
        const wchar_t* lpwstr = pathString.c_str();

        auto handle = FindFirstChangeNotificationW(lpwstr, false, FILE_NOTIFY_CHANGE_LAST_WRITE);
        
        if (handle == INVALID_HANDLE_VALUE)
        {
            *retValue = GetLastError();
            watcherRunning = false;
        }

        // Use RAII to close handle.
        auto ptr = std::unique_ptr<void, std::function<BOOL(void*)>>(handle, FindCloseChangeNotification);

        waitFlag->test_and_set();
        waitFlag->notify_one();

        logger.info(L"Waiting for directory changes (path: '" + _path.wstring() + L"')");

        while (watcherRunning)
        {    
            WaitForSingleObjectEx(handle, INFINITE, true);
        
            logger.info(L"Change detected !");

            // Find next changes:
            if (!FindNextChangeNotification(handle))
            {
                logger.error(L"FindNextChangeNotification returned FALSE, exiting thread.");
                watcherRunning = false;

                return;
            }

            findChanges();
        }
    }
}
