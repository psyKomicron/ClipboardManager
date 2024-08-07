#include "pch.h"
#include "ClipboardTrigger.hpp"

#include "utils/helpers.hpp"

#include <boost/property_tree/xml_parser.hpp>

#include <iostream>

clip::ClipboardTrigger::ClipboardTrigger(const std::wstring& label, const std::wstring& format, const boost::wregex& regex, const bool& enabled) :
    _label{ label },
    _format{ format },
    _regex{ regex },
    _enabled{ enabled }
{
}

std::vector<clip::ClipboardTrigger> clip::ClipboardTrigger::loadClipboardTriggers(const std::filesystem::path& userFilePath)
{
    if (utils::pathExists(userFilePath))
    {
        std::vector<clip::ClipboardTrigger> urls{};
        boost::property_tree::wptree tree{};

        try
        {
            boost::property_tree::read_xml(userFilePath.string(), tree);
            
            for (auto&& child : tree.get_child(L"settings.triggers"))
            {
                auto&& node = child.second;
                auto&& url = node.get_child(L"format").data();
                auto&& label = node.get_child(L"label").data();
                auto&& enabled = node.get_child(L"enabled").data().compare(L"true") == 0;

                auto&& regexNode = node.get_child(L"re");
                auto&& ignoreCaseAttribute = regexNode.get_child_optional(L"<xmlattr>.ignoreCase");
                auto&& modeAttribute = regexNode.get_child_optional(L"<xmlattr>.mode");

                bool ignoreCase = ignoreCaseAttribute.has_value() && (ignoreCaseAttribute.get().data() == L"1" || ignoreCaseAttribute.get().data() == L"true");
                auto flags = boost::regex_constants::normal;
                if (ignoreCase)
                {
                    flags = boost::regex_constants::icase;
                }

                std::optional<MatchMode> triggerMatchMode{};
                if (modeAttribute.has_value())
                {
                    bool useSearch = modeAttribute.get().data() == L"search";
                    triggerMatchMode = useSearch ? MatchMode::Search : MatchMode::Match;
                }
                
                auto clipboardTrigger = clip::ClipboardTrigger(label, url, boost::wregex(regexNode.data(), flags), enabled);
                clipboardTrigger._matchMode = triggerMatchMode;

                urls.push_back(std::move(clipboardTrigger));
            }
        }
        catch (const boost::property_tree::xml_parser_error& parserError)
        {
            if (parserError.line() == 0) // This should mean that the error is at the beginning of the file, thus it's empty.
            {
                firstTimeInitialization(userFilePath, tree);
            }
            else
            {
                throw;
            }
        }
        
        return urls;
    }
    else
    {
        throw std::runtime_error("File path doesn't exist.");
    }
}

void clip::ClipboardTrigger::saveClipboardTriggers(const std::vector<ClipboardTrigger>& triggersList, const std::filesystem::path& userFilePath)
{
    if (utils::pathExists(userFilePath))
    {
        boost::property_tree::wptree tree{};
        boost::property_tree::read_xml(userFilePath.string(), tree);

        // Erase the tree so I dont induce conflicts.
        auto settingsNode = tree.get_child_optional(L"settings");
        if (settingsNode.has_value())
        {
            settingsNode.value().erase(L"triggers");
            
            auto&& triggersNode = tree.put(L"settings.triggers", L"");

            for (auto&& trigger : triggersList)
            {
                auto&& triggerNode = triggersNode.add(L"trigger", L"");

                auto&& re = triggerNode.add(L"re", trigger.regex().str());

                auto flags = trigger._regex.flags();
                re.add(L"<xmlattr>.ignoreCase", ((flags & boost::regex_constants::icase) == boost::regex_constants::icase ? L"true" : L"false"));
                re.add(L"<xmlattr>.mode", (trigger._matchMode == MatchMode::Match ? L"match" : L"search"));

                triggerNode.add(L"format", trigger.format());
                triggerNode.add(L"label", trigger.label());
                triggerNode.add(L"enabled", trigger.enabled());
            }

            boost::property_tree::write_xml(userFilePath.string(), tree);
        }
    }
    else
    {
        throw std::runtime_error("File path doesn't exist.");
    }
}


void clip::ClipboardTrigger::firstTimeInitialization(const std::filesystem::path& path, boost::property_tree::wptree tree)
{
    auto&& actions = tree.put(L"settings.triggers", L"");
    actions.add(L"trigger.re", L"[A-Z]{,3}");
    actions.add(L"trigger.format", L"https://example-namespace.com/search?={}");
    actions.add(L"trigger.label", L"Search example-namespace");
    actions.add(L"trigger.enabled", L"true");

    boost::property_tree::write_xml(path.string(), tree);
}

void clip::ClipboardTrigger::initializeSaveFile(const std::filesystem::path& userFilePath)
{
    if (clip::utils::pathExists(userFilePath) && clip::utils::pathExists(userFilePath.parent_path()))
    {
        boost::property_tree::wptree tree{};
        firstTimeInitialization(userFilePath, tree);
    }
    else
    {
        std::wcerr << L"[ClipboardTrigger]   Can't create user file, it either already exists or it's parent folder doesn't." << std::endl;
    }
}

std::wstring clip::ClipboardTrigger::label() const
{
    return _label;
}

void clip::ClipboardTrigger::label(const std::wstring& value)
{
    _label = value;
}

std::wstring clip::ClipboardTrigger::format() const
{
    return _format;
}

void clip::ClipboardTrigger::format(const std::wstring& value)
{
    _format = value;
}

boost::wregex clip::ClipboardTrigger::regex() const
{
    return _regex;
}

void clip::ClipboardTrigger::regex(const boost::wregex& regex)
{
    _regex = regex;
}

bool clip::ClipboardTrigger::enabled() const
{
    return _enabled;
}

void clip::ClipboardTrigger::enabled(const bool& value)
{
    _enabled = value;
}

std::optional<clip::MatchMode> clip::ClipboardTrigger::matchMode() const
{
    return _matchMode;
}

void clip::ClipboardTrigger::updateMatchMode(const MatchMode& mode)
{
    _matchMode = mode;
}

bool clip::ClipboardTrigger::match(const std::wstring& string, const std::optional<MatchMode>& defaultMatchMode) const
{
    auto matchMode = _matchMode.has_value()
        ? _matchMode.value()
        : defaultMatchMode.value_or(MatchMode::Match);
    
    return (matchMode == MatchMode::Match && boost::regex_match(string, _regex))
        || (matchMode == MatchMode::Search && boost::regex_search(string, _regex));
}

bool clip::ClipboardTrigger::operator==(ClipboardTrigger& other)
{
    return _enabled == other.enabled()
        &&_label == other.label()
        && _format == other.format()
        && _regex == other.regex();
}
