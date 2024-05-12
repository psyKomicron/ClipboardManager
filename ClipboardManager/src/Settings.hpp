#pragma once
#include "ClipboardAction.hpp"

#include <winrt/Windows.Storage.h>

#include <boost/property_tree/ptree.hpp>

#include <string>
#include <vector>
#include <filesystem>

namespace clipmgr
{
    class Settings
    {
    public:
        Settings() = default;

        void open();

        std::vector<ClipboardAction> getClipboardActions();

    private:
        const std::wstring APPLICATION_NAME = L"Clipboard Manager";
        const std::wstring DEFAULT_USER_FILE_NAME = L"user_file.xml";
        boost::property_tree::wptree tree{};
        bool empty = true;

        std::filesystem::path getDefaultUserFileFolder() const;
        void firstTimeInitialization(const std::filesystem::path& userFilePath);
    };
}

