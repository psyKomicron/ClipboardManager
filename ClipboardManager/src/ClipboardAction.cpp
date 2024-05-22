#include "pch.h"
#include "ClipboardAction.hpp"

#include "utils/helpers.hpp"

#include <boost/property_tree/xml_parser.hpp>

#include <iostream>

clipmgr::ClipboardAction::ClipboardAction(const std::wstring& label, const std::wstring& format, const boost::wregex& regex, const bool& enabled) :
    _label{ label },
    _format{ format },
    _regex{ regex },
    _enabled{ enabled }
{
}

std::vector<clipmgr::ClipboardAction> clipmgr::ClipboardAction::loadClipboardActions(const std::filesystem::path& userFilePath)
{
    if (utils::pathExists(userFilePath))
    {
        std::vector<clipmgr::ClipboardAction> urls{};
        boost::property_tree::wptree tree{};

        try
        {
            std::wcout << L"[ClipboardAction]   Reading user clipboard actions file '" << userFilePath.wstring() << L"'" << std::endl;
            boost::property_tree::read_xml(userFilePath.string(), tree);

            for (auto&& node : tree.get_child(L"settings.actions"))
            {
                auto&& url = node.second.get_child(L"format").data();
                auto&& label = node.second.get_child(L"label").data();
                auto&& enabled = node.second.get_child(L"enabled").data().compare(L"true") == 0;

                boost::wregex regex{};
                auto&& regexNode = node.second.get_child(L"re");
                if (regexNode.get_child_optional(L"options").has_value())
                {
                    auto&& options = node.second.get_child(L"options");
                    try
                    {
                        uint64_t flags = std::stoull(options.data());
                        regex = boost::wregex(regex.str(), static_cast<boost::regex_constants::syntax_option_type>(flags));
                    }
                    catch (std::invalid_argument)
                    {
                        // TODO: Implement light parsing of strings to get regex options.
                        /* Options letters:
                        * - b: basic
                        * - e: extended
                        * - n: normal
                        * - g: grep
                        * - i: icase
                        */
                        /*boost::wregex expectedFormat{ L"[bengi]" };
                        if (boost::regex_search(options.data(), expectedFormat))
                        {

                        }*/
                    }
                }
                else
                {
                    regex = boost::wregex(regexNode.data());
                }

                urls.push_back(clipmgr::ClipboardAction(label, url, regex, enabled));
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

void clipmgr::ClipboardAction::firstTimeInitialization(const std::filesystem::path& path, boost::property_tree::wptree tree)
{
    std::wcout << L"[ClipboardAction]   First time initialization of user file.\n";
    auto&& actions = tree.put(L"settings.actions", L"");
    actions.add(L"action.re", L"[A-Z]{,3}");
    actions.add(L"action.format", L"https://example-namespace.com/search?={}");
    actions.add(L"action.label", L"Search example-namespace");
    actions.add(L"action.enabled", L"true");

    tree.put(L"settings.preferences", L"");

    boost::property_tree::write_xml(path.string(), tree);
}

void clipmgr::ClipboardAction::initializeSaveFile(const std::filesystem::path& userFilePath)
{
    if (clipmgr::utils::pathExists(userFilePath) && clipmgr::utils::pathExists(userFilePath.parent_path()))
    {
        boost::property_tree::wptree tree{};
        firstTimeInitialization(userFilePath, tree);
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

bool clipmgr::ClipboardAction::enabled() const
{
    return _enabled;
}

bool clipmgr::ClipboardAction::match(const std::wstring& string) const
{
    bool res = boost::regex_search(string, _regex);
    return res;
}