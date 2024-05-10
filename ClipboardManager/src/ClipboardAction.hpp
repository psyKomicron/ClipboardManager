#pragma once
#include <boost/regex.hpp>

namespace clipmgr
{
    class ClipboardAction
    {
    public:
        ClipboardAction(const std::wstring& label, const std::wstring& format, const std::wstring& regexString);

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

