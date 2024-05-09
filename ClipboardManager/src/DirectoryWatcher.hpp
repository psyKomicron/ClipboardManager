#pragma once
#include <filesystem>

namespace clipmgr
{
    class DirectoryWatcher
    {
    public:
        DirectoryWatcher(const std::filesystem::path& path);
        ~DirectoryWatcher();

        void startWatching();
        void stopWatching();
    };
}

