#include "pch.h"
#include "ClipboardAction.hpp"

#include "utils/helpers.hpp"

#include <boost/property_tree/xml_parser.hpp>

#include <iostream>

clipmgr::ClipboardAction::ClipboardAction(const std::wstring& label, const std::wstring& format, const std::wstring& regexString) :
    _label{ label },
    _format{ format },
    _regex{ createRegex(regexString) }
{

}

std::vector<clipmgr::ClipboardAction> clipmgr::ClipboardAction::loadClipboardActions(const std::filesystem::path& userFilePath)
{
    if (utils::pathExists(userFilePath))
    {
        boost::property_tree::wptree tree{};
        std::vector<clipmgr::ClipboardAction> urls{};
        try
        {

            std::wcout << L"[ClipboardAction]   Reading user clipboard actions file '" << userFilePath.wstring() << L"'" << std::endl;
            boost::property_tree::read_xml(userFilePath.string(), tree);

            for (auto&& node : tree.get_child(L"settings.actions"))
            {
                auto&& url = node.second.get_child(L"format").data();
                auto&& regex = node.second.get_child(L"re").data();
                auto&& label = node.second.get_child(L"label").data();

                urls.push_back(clipmgr::ClipboardAction(label, url, regex));
            }
        }
        catch (const boost::property_tree::xml_parser_error& parserError)
        {
            std::wcout << L"[ClipboardAction]   Xml parser error '" << userFilePath.wstring() << L"': " << std::endl;
            if (parserError.line() == 0) // This should mean that the error is at the beginning of the file, thus it's empty.
            {
                firstTimeInitialization(userFilePath, tree);
            }
        }
        return urls;
    }
    else
    {
        throw std::runtime_error("File path doesn't exist.");
    }
}

void clipmgr::ClipboardAction::createSaveFile(const std::filesystem::path& userFilePath)
{
    boost::property_tree::wptree tree{};

    if (!clipmgr::utils::pathExists(userFilePath) && clipmgr::utils::pathExists(userFilePath.parent_path()))
    {
        std::ignore = clipmgr::utils::createFile(userFilePath);
        firstTimeInitialization(userFilePath, tree);
        std::wcout << L"[ClipboardAction]   " << std::quoted(userFilePath.wstring()) << L" User file created." << std::endl;
    }
    else
    {
        std::wcerr << L"[ClipboardAction]   Can't create user file, it either already exists or it's parent folder doesn't." << std::endl;
    }
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

void clipmgr::ClipboardAction::firstTimeInitialization(const std::filesystem::path& path, boost::property_tree::wptree tree)
{
    std::wcout << L"[ClipboardAction]   First time initialization of user file.\n";
    auto&& actions = tree.put(L"settings.actions", L"");
    actions.add(L"action.re", L"[A-Z]{,3}");
    actions.add(L"action.format", L"https://example-namespace.com/search?={}");
    actions.add(L"action.label", L"Search example-namespace");

    tree.put(L"settings.preferences", L"");

    boost::property_tree::write_xml(path.string(), tree);
}

boost::wregex clipmgr::ClipboardAction::createRegex(const std::wstring& string)
{
    //TODO: Parse regex for options.
    return boost::wregex(string);
}