#pragma once
#include <boost/regex.hpp>
#include <boost/property_tree/ptree.hpp>

#include <vector>
#include <filesystem>

namespace clipmgr
{
    class ClipboardAction
    {
    public:
        ClipboardAction(const std::wstring& label, const std::wstring& format, const boost::wregex& regex, const bool& enabled);

        static std::vector<ClipboardAction> loadClipboardActions(const std::filesystem::path& userFilePath);
        static void initializeSaveFile(const std::filesystem::path& userFilePath);

        std::wstring label() const;
        std::wstring format() const;
        boost::wregex regex() const;
        bool enabled() const;

        bool match(const std::wstring& string) const;

    private:
        std::wstring _label{};
        std::wstring _format{};
        boost::wregex _regex{};
        bool _enabled = true;

        static void firstTimeInitialization(const std::filesystem::path& path, boost::property_tree::wptree tree);
    };
}

