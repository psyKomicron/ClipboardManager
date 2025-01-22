#pragma once
#include "lib/utils/Logger.hpp"

#include <filesystem>
#include <thread>
#include <functional>

#include "lib/murmur/MurmurHash3.hpp"

namespace clip
{
    class FileWatcher
    {
    public:
        FileWatcher(const std::function<void()>& event);

        std::filesystem::path path() const;

        void startWatching(const std::filesystem::path& path);
        void stopWatching();

    private:
        clip::utils::Logger logger{ L"FileWatcher" };
        std::atomic_bool watcherRunning = false;
        std::jthread backgroundThread{};
        std::function<void()> eventCallback{};
        std::pair<uint64_t, uint64_t> lastHash{};
        std::filesystem::path _path{};
        const clip::murmur::MurmurHash3 hasher{};

        void findChanges();
        void watchDirectoryChanges(std::atomic_flag* waitFlag, int* retValue);
    };
}

