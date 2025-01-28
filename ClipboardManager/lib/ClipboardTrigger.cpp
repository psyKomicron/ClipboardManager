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
    }

    ClipboardTrigger::ClipboardTrigger(boost::property_tree::wptree& node)
    {
        _label = node.get_child(L"label").data();
        _enabled = node.get_child(L"enabled").data().compare(L"true") == 0;

        auto&& formatNode = node.get_child(L"format");
        _format = formatNode.data();
        auto&& useRegexMatchResultsAttribute = formatNode.get_child_optional(L"<xmlattr>.useRegexMatchResults");
        _useRegexMatchResults = useRegexMatchResultsAttribute.has_value() && useRegexMatchResultsAttribute.value().get_value(false);

        auto&& regexNode = node.get_child(L"re");
        auto&& ignoreCaseAttribute = regexNode.get_child_optional(L"<xmlattr>.ignoreCase");
        auto&& modeAttribute = regexNode.get_child_optional(L"<xmlattr>.mode");

        bool ignoreCase = ignoreCaseAttribute.has_value() && ignoreCaseAttribute.value().get_value(false);
        auto flags = ignoreCase ? boost::regex_constants::icase : boost::regex_constants::normal;
        _regex = boost::wregex(regexNode.data(), flags);

        if (modeAttribute.has_value())
        {
            bool useSearch = modeAttribute.get().data() == L"search";
            _matchMode = useSearch ? MatchMode::Search : MatchMode::Match;
        }
    }


    std::vector<ClipboardTrigger> ClipboardTrigger::loadClipboardTriggers(const std::filesystem::path& userFilePath)
    {
        if (utils::pathExists(userFilePath))
        {
            std::vector<ClipboardTrigger> urls{};

            boost::property_tree::wptree tree{};
            boost::property_tree::read_xml(userFilePath.string(), tree, boost::property_tree::xml_parser::trim_whitespace);

            for (auto&& child : tree.get_child(L"settings.triggers"))
            {
                auto&& node = child.second;
                urls.push_back(std::move(ClipboardTrigger(node)));
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
        if (std::filesystem::exists(userFilePath))
        {
            boost::property_tree::wptree tree{};
            boost::property_tree::read_xml(userFilePath.string(), tree, boost::property_tree::xml_parser::trim_whitespace);

            // Erase the tree so I dont induce conflicts.
            auto settingsNode = tree.get_child_optional(L"settings");
            if (settingsNode.has_value())
            {
                auto&& triggersNode = tree.put_child(L"settings.triggers", boost::property_tree::wptree());

                // For each trigger, create an empty trigger node for the trigger object to save it-self.
                for (auto&& trigger : triggersList)
                {
                    triggersNode.push_front({ L"trigger", trigger.serialize() });
                }

                boost::property_tree::write_xml(userFilePath.string(), tree, std::locale(), boost::property_tree::xml_writer_settings<std::wstring>('\t', 1));
            }
        }
        else
        {
            throw std::runtime_error("File path doesn't exist.");
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

    bool ClipboardTrigger::useRegexMatchResults() const
    {
        return _useRegexMatchResults;
    }

    void ClipboardTrigger::useRegexMatchResults(const bool& value)
    {
        _useRegexMatchResults = value;
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

    void ClipboardTrigger::checkFormat() const
    {
        auto optional = checkFormat(_format);
        if (optional.has_value())
        {
            throw optional.value();
        }
    }

    std::wstring ClipboardTrigger::formatTrigger(const std::wstring& string)
    {
        if (_useRegexMatchResults)
        {
            bool result = false;
            boost::wsmatch matchResults{};

            if (_matchMode.value_or(MatchMode::Match) == MatchMode::Match)
            {
                result = boost::regex_match(string, matchResults, _regex);
            }
            else
            {
                result = boost::regex_search(string, matchResults, _regex);
            }

            if (result && matchResults.size() > 1)
            {
                auto str = matchResults[1].str();
                return std::vformat(_format, std::make_wformat_args(str));
            }
            else
            {
                logger.debug(L"Failed to format string with regex match results.");
            }
        }

        return std::vformat(_format, std::make_wformat_args(string));
    }

    bool ClipboardTrigger::operator==(ClipboardTrigger& other)
    {
        return _enabled == other.enabled()
            && _label == other.label()
            && _format == other.format()
            && _regex == other.regex();
    }


    std::optional<ClipboardTriggerFormatException> ClipboardTrigger::checkFormat(const std::wstring& format) const
    {
        if (format.empty())
        {
            return ClipboardTriggerFormatException(FormatExceptionCode::EmptyString);
        }

        using string_t = std::wstring;
        size_t openIndex = format.find_first_of(L'{');
        size_t closeIndex = format.find_first_of(L'}');

        if (openIndex == string_t::npos && closeIndex == string_t::npos)
        {
            // Missing both '{' and '}'
            auto flags = FormatExceptionCode::MissingOpenBraces | FormatExceptionCode::MissingClosingBraces;
            return ClipboardTriggerFormatException(flags);
        }
        else if (openIndex == string_t::npos)
        {
            // Missing '{'
            return ClipboardTriggerFormatException(FormatExceptionCode::MissingOpenBraces);
        }
        else if (closeIndex == string_t::npos)
        {
            // Missing '}'
            return ClipboardTriggerFormatException(FormatExceptionCode::MissingClosingBraces);
        }

        while (openIndex != std::string::npos)
        {
            closeIndex = format.find_first_of(L'}', openIndex);
            if (closeIndex != std::string::npos)
            {
                if ((closeIndex - ++openIndex) > 0)
                {
                    // Check validity of the content between the 2 braces:
                    auto bracesContent = format.substr(openIndex, closeIndex - openIndex);
                    try
                    {
                        if (std::stoi(bracesContent) > 0)
                        {
                            // Invalid argument, braces content should always be 0.
                            return ClipboardTriggerFormatException(FormatExceptionCode::ArgumentNotFound | FormatExceptionCode::OutOfRangeFormatArgument);
                        }
                    }
                    catch (std::invalid_argument)
                    {
                        // Braces content is not a integral value.
                        return ClipboardTriggerFormatException(FormatExceptionCode::InvalidFormatArgument);
                    }
                }
            }
            else
            {
                return ClipboardTriggerFormatException(FormatExceptionCode::MissingClosingBraces | FormatExceptionCode::BadBraceOrder);
            }

            openIndex = format.find_first_of(L'{', closeIndex);
        }

        closeIndex = format.find_first_of(L'}', ++closeIndex);
        if (closeIndex != std::string::npos)
        {
            return ClipboardTriggerFormatException(FormatExceptionCode::MissingOpenBraces | FormatExceptionCode::BadBraceOrder);
        }

        try
        {
            // Use std's format to check if the format is valid. Previous checks do not account for all possible errors.
            std::ignore = std::vformat(format, std::make_wformat_args(L"ClipboardManager"));
        }
        catch (std::format_error& formatError)
        {
            const std::string what{ formatError.what() };

            return what.compare("Argument not found.") == 0
                ? ClipboardTriggerFormatException(FormatExceptionCode::ArgumentNotFound)
                : ClipboardTriggerFormatException(FormatExceptionCode::InvalidFormat, clip::utils::to_wstring(formatError.what()));
        }

        return {};
    }

    boost::property_tree::wptree ClipboardTrigger::serialize() const
    {
        boost::property_tree::wptree triggerNode{};
        triggerNode.add(L"label", _label);
        triggerNode.add(L"enabled", _enabled);

        auto flags = _regex.flags();
        auto&& reNode = triggerNode.add(L"re", _regex.str());
        reNode.add(L"<xmlattr>.ignoreCase", ((flags & boost::regex_constants::icase) == boost::regex_constants::icase ? L"true" : L"false"));
        reNode.add(L"<xmlattr>.mode", (_matchMode == MatchMode::Match ? L"match" : L"search"));

        auto&& formatNode = triggerNode.add(L"format", _format);
        formatNode.add(L"<xmlattr>.useRegexMatchResults", _useRegexMatchResults);

        return triggerNode;
    }
}
