#include "pch.h"
#include "ClipboardTrigger.hpp"

#include "utils/helpers.hpp"

#include <boost/property_tree/xml_parser.hpp>

#include <iostream>

clipmgr::ClipboardTrigger::ClipboardTrigger(const std::wstring& label, const std::wstring& format, const boost::wregex& regex, const bool& enabled) :
    _label{ label },
    _format{ format },
    _regex{ regex },
    _enabled{ enabled }
{
}

std::vector<clipmgr::ClipboardTrigger> clipmgr::ClipboardTrigger::loadClipboardActions(const std::filesystem::path& userFilePath)
{
    if (utils::pathExists(userFilePath))
    {
        std::vector<clipmgr::ClipboardTrigger> urls{};
        boost::property_tree::wptree tree{};

        try
        {
            std::wcout << L"[ClipboardTrigger]   Reading user clipboard actions file '" << userFilePath.wstring() << L"'" << std::endl;
            boost::property_tree::read_xml(userFilePath.string(), tree);

            for (auto&& child : tree.get_child(L"settings.actions"))
            {
                auto&& node = child.second;
                auto&& url = node.get_child(L"format").data();
                auto&& label = node.get_child(L"label").data();
                auto&& enabled = node.get_child(L"enabled").data().compare(L"true") == 0;

                auto&& regexNode = node.get_child(L"re");
                auto&& ignoreCaseAttribute = regexNode.get_child_optional(L"<xmlattr>.ignoreCase");
                auto&& modeAttribute = regexNode.get_child_optional(L"<xmlattr>.mode");

                bool ignoreCase = ignoreCaseAttribute.has_value() && (ignoreCaseAttribute.get().data() == L"1" || ignoreCaseAttribute.get().data() == L"true");
                bool useSearch = modeAttribute.has_value() && modeAttribute.get().data() == L"search";
                
                auto flags = boost::regex_constants::normal;
                
                if (ignoreCase)
                {
                    flags = boost::regex_constants::icase;
                }
                
                auto clipboardTrigger = clipmgr::ClipboardTrigger(label, url, boost::wregex(regexNode.data(), flags), enabled);
                clipboardTrigger._matchMode = useSearch ? MatchMode::Search : MatchMode::Match;

                urls.push_back(std::move(clipboardTrigger));
            }
        }
        catch (const boost::property_tree::xml_parser_error& parserError)
        {
            std::wcout << L"[ClipboardTrigger]   Xml parser error '" << userFilePath.wstring() << L"': " << std::endl;
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


void clipmgr::ClipboardTrigger::firstTimeInitialization(const std::filesystem::path& path, boost::property_tree::wptree tree)
{
    std::wcout << L"[ClipboardTrigger]   First time initialization of user file.\n";
    auto&& actions = tree.put(L"settings.actions", L"");
    actions.add(L"action.re", L"[A-Z]{,3}");
    actions.add(L"action.format", L"https://example-namespace.com/search?={}");
    actions.add(L"action.label", L"Search example-namespace");
    actions.add(L"action.enabled", L"true");

    tree.put(L"settings.preferences", L"");

    boost::property_tree::write_xml(path.string(), tree);
}

void clipmgr::ClipboardTrigger::initializeSaveFile(const std::filesystem::path& userFilePath)
{
    if (clipmgr::utils::pathExists(userFilePath) && clipmgr::utils::pathExists(userFilePath.parent_path()))
    {
        boost::property_tree::wptree tree{};
        firstTimeInitialization(userFilePath, tree);
    }
    else
    {
        std::wcerr << L"[ClipboardTrigger]   Can't create user file, it either already exists or it's parent folder doesn't." << std::endl;
    }
}

std::wstring clipmgr::ClipboardTrigger::label() const
{
    return _label;
}

void clipmgr::ClipboardTrigger::label(const std::wstring& value)
{
    _label = value;
}

std::wstring clipmgr::ClipboardTrigger::format() const
{
    return _format;
}

void clipmgr::ClipboardTrigger::format(const std::wstring& value)
{
    _format = value;
}

boost::wregex clipmgr::ClipboardTrigger::regex() const
{
    return _regex;
}

void clipmgr::ClipboardTrigger::regex(const boost::wregex& regex)
{
    _regex = regex;
}

bool clipmgr::ClipboardTrigger::enabled() const
{
    return _enabled;
}

void clipmgr::ClipboardTrigger::enabled(const bool& value)
{
    _enabled = value;
}

void clipmgr::ClipboardTrigger::updateMatchMode(const MatchMode& mode)
{
    _matchMode = mode;
}

bool clipmgr::ClipboardTrigger::match(const std::wstring& string) const
{
    return (_matchMode == MatchMode::Match && boost::regex_match(string, _regex))
        || (_matchMode == MatchMode::Search && boost::regex_search(string, _regex));
}

bool clipmgr::ClipboardTrigger::operator==(ClipboardTrigger& other)
{
    return _enabled == other.enabled()
        &&_label == other.label()
        && _format == other.format()
        && _regex == other.regex();
}
