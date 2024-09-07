#include "pch.h"
#include "ClipboardTrigger.hpp"

#include "utils/helpers.hpp"

#include <boost/property_tree/xml_parser.hpp>

#include <iostream>
#include <format>
#include <string>

namespace clip
{
    ClipboardTriggerFormatException::ClipboardTriggerFormatException(const FormatExceptionCode& code, const std::wstring& message) :
        std::invalid_argument("Invalid format"),
        _message{ message },
        _code{ code }
    {
    }

    std::wstring ClipboardTriggerFormatException::message() const
    {
        return _message;
    }

    FormatExceptionCode ClipboardTriggerFormatException::code() const
    {
        return _code;
    }
}

namespace clip
{
    ClipboardTrigger::ClipboardTrigger(const std::wstring& label, const std::wstring& format, const boost::wregex& regex, const bool& enabled) :
        _label{ label },
        _format{ format },
        _regex{ regex },
        _enabled{ enabled }
    {
        //checkFormat();
    }

    std::vector<ClipboardTrigger> ClipboardTrigger::loadClipboardTriggers(const std::filesystem::path& userFilePath)
    {
        if (utils::pathExists(userFilePath))
        {
            std::vector<ClipboardTrigger> urls{};
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
                    auto flags = ignoreCase ? boost::regex_constants::icase : boost::regex_constants::normal;
                    
                    std::optional<MatchMode> triggerMatchMode{};
                    if (modeAttribute.has_value())
                    {
                        bool useSearch = modeAttribute.get().data() == L"search";
                        triggerMatchMode = useSearch ? MatchMode::Search : MatchMode::Match;
                    }
                
                    auto clipboardTrigger = ClipboardTrigger(label, url, boost::wregex(regexNode.data(), flags), enabled);
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

    void ClipboardTrigger::saveClipboardTriggers(const std::vector<ClipboardTrigger>& triggersList, const std::filesystem::path& userFilePath)
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


    void ClipboardTrigger::initializeSaveFile(const std::filesystem::path& userFilePath)
    {
        if (utils::pathExists(userFilePath) && utils::pathExists(userFilePath.parent_path()))
        {
            boost::property_tree::wptree tree{};
            firstTimeInitialization(userFilePath, tree);
        }
        else
        {
            std::wcerr << L"[ClipboardTrigger]   Can't create user file, it either already exists or it's parent folder doesn't." << std::endl;
        }
    }

    std::wstring ClipboardTrigger::label() const
    {
        return _label;
    }

    void ClipboardTrigger::label(const std::wstring& value)
    {
        _label = value;
    }

    std::wstring ClipboardTrigger::format() const
    {
        return _format;
    }

    void ClipboardTrigger::format(const std::wstring& value)
    {
        checkFormat(value);
        _format = value;
    }

    boost::wregex ClipboardTrigger::regex() const
    {
        return _regex;
    }

    void ClipboardTrigger::regex(const boost::wregex& regex)
    {
        _regex = regex;
    }

    bool ClipboardTrigger::enabled() const
    {
        return _enabled;
    }

    void ClipboardTrigger::enabled(const bool& value)
    {
        _enabled = value;
    }

    std::optional<MatchMode> ClipboardTrigger::matchMode() const
    {
        return _matchMode;
    }

    void ClipboardTrigger::updateMatchMode(const MatchMode& mode)
    {
        _matchMode = mode;
    }

    bool ClipboardTrigger::match(const std::wstring& string, const std::optional<MatchMode>& defaultMatchMode) const
    {
        auto matchMode = _matchMode.has_value()
            ? _matchMode.value()
            : defaultMatchMode.value_or(MatchMode::Match);
    
        return (matchMode == MatchMode::Match && boost::regex_match(string, _regex))
            || (matchMode == MatchMode::Search && boost::regex_search(string, _regex));
    }

    void ClipboardTrigger::checkFormat()
    {
        throw checkFormat(_format);
    }

    bool ClipboardTrigger::operator==(ClipboardTrigger& other)
    {
        return _enabled == other.enabled()
            &&_label == other.label()
            && _format == other.format()
            && _regex == other.regex();
    }


    void ClipboardTrigger::firstTimeInitialization(const std::filesystem::path& path, boost::property_tree::wptree tree)
    {
        auto&& actions = tree.put(L"settings.triggers", L"");
        actions.add(L"trigger.re", L"[A-Z]{,3}");
        actions.add(L"trigger.format", L"https://example-namespace.com/search?={}");
        actions.add(L"trigger.label", L"Search example-namespace");
        actions.add(L"trigger.enabled", L"true");

        boost::property_tree::write_xml(path.string(), tree);
    }

    ClipboardTriggerFormatException ClipboardTrigger::checkFormat(const std::wstring& format)
    {
        const auto npos = std::wstring::npos;

        auto openIndex = format.find_first_of(L'{');
        auto closeIndex = format.find_first_of(L'}');

        if (openIndex == npos && closeIndex == npos)
        {
            // Missing both '{' and '}'
            auto flags = FormatExceptionCode::MissingOpenBraces | FormatExceptionCode::MissingClosingBraces;
            return ClipboardTriggerFormatException(flags);
        }
        else if (openIndex == npos)
        {
            // Missing '{'
            return ClipboardTriggerFormatException(FormatExceptionCode::MissingOpenBraces);
        }
        else
        {
            // Missing '}'
            return ClipboardTriggerFormatException(FormatExceptionCode::MissingClosingBraces);
        }

        //if (openIndex != (closeIndex - 1))
        //{
        //    if (openIndex > closeIndex)
        //    {
        //        // }{
        //    }

        //    auto openIndexN1 = format.at(openIndex + 1);
        //    
        //}

        try
        {
            std::ignore = std::vformat(format, std::make_wformat_args(L"ClipboardManager"));
        }
        catch (std::format_error& formatError)
        {
            return ClipboardTriggerFormatException(FormatExceptionCode::InvalidFormat, clip::utils::convert(formatError.what()));
        }
    }
}
