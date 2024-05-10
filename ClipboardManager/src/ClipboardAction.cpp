#include "pch.h"
#include "ClipboardAction.hpp"

clipmgr::ClipboardAction::ClipboardAction(const std::wstring& label, const std::wstring& format, const std::wstring& regexString) :
    _label{ label },
    _format{ format },
    _regex{ createRegex(regexString) }
{

}

std::wstring clipmgr::ClipboardAction::label() const
{
    return _label;
}

std::wstring clipmgr::ClipboardAction::format() const
{
    return _format;
}

boost::wregex clipmgr::ClipboardAction::regex() const
{
    return _regex;
}

bool clipmgr::ClipboardAction::match(const std::wstring& string)
{
    bool res = boost::regex_search(string, _regex);
    return res;
}

boost::wregex clipmgr::ClipboardAction::createRegex(const std::wstring& string)
{
    //TODO: Parse regex for options.
    return boost::wregex(string);
}