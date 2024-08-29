#pragma once
#include "src/utils/Logger.hpp"

#include <filesystem>
#include <thread>

namespace clip
{
    class DirectoryWatcher
    {
    public:
        DirectoryWatcher(const std::filesystem::path& path);
        ~DirectoryWatcher();

        void startWatching();
        void stopWatching();

    private:
        std::jthread backgroundThread{};
        std::atomic_bool watcherRunning = false;
        std::atomic_bool runThread = true;
        std::filesystem::path path{};
        clip::utils::Logger logger{ L"DirectoryWatcher" };

        void watchDirectoryChanges(std::atomic_flag* waitFlag, int* retValue);
    };
}

