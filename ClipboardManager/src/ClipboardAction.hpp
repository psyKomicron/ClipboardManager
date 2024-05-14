#pragma once
#include <boost/regex.hpp>

#include <vector>
#include <filesystem>

namespace clipmgr
{
    class ClipboardAction
    {
    public:
        ClipboardAction(const std::wstring& label, const std::wstring& format, const std::wstring& regexString);

        static std::vector<ClipboardAction> loadClipboardActions(const std::filesystem::path& userFilePath);

        std::wstring label() const;
        std::wstring format() const;
        boost::wregex regex() const;

        bool match(const std::wstring& string);

    private:
        std::wstring _label{};
        std::wstring _format{};
        boost::wregex _regex{};

        boost::wregex createRegex(const std::wstring& string);
    };
}

