#pragma once
#include "lib/utils/Logger.hpp"
#include "lib/FormatExceptionCode.hpp"

#include <boost/regex.hpp>
#include <boost/property_tree/ptree.hpp>

#include <vector>
#include <filesystem>
#include <stdexcept>

namespace clip
{
    enum class MatchMode
    {
        Match = 0,
        Search = 1
    };

    class ClipboardTriggerFormatException : std::invalid_argument
    {
    public:
        ClipboardTriggerFormatException(const FormatExceptionCode& code, const std::wstring& message = {});

        std::wstring message() const;
        FormatExceptionCode code() const;

    private:
        std::wstring _message{};
        FormatExceptionCode _code = FormatExceptionCode::Unknown;
    };

    class ClipboardTrigger
    {
    public:
        ClipboardTrigger(const std::wstring& label, const std::wstring& format, const boost::wregex& regex, const bool& enabled);

        static std::vector<ClipboardTrigger> loadClipboardTriggers(const std::filesystem::path& userFilePath);
        /**
         * @brief Saves an xml representation of the list of clipboard triggers to an xml file pointed by 'userFilePath'.
         * @param triggers List of clipboard triggers to save.
         * @param userFilePath Path to the xml file.
         * @throws boost::property_tree::xml_parser_error - Thrown when the xml parser fails to perform an action.
         * @throws std::runtime_error - Thrown if the path pointed by 'userFilePath' is invalid.
         */
        static void saveClipboardTriggers(const std::vector<ClipboardTrigger>& triggers, const std::filesystem::path& userFilePath);

        std::wstring label() const;
        void label(const std::wstring& value);
        std::wstring format() const;
        void format(const std::wstring& value);
        boost::wregex regex() const;
        void regex(const boost::wregex& regex);
        bool enabled() const;
        void enabled(const bool& value);
        std::optional<clip::MatchMode> matchMode() const;
        bool useRegexMatchResults() const;
        void useRegexMatchResults(const bool& value);

        void updateMatchMode(const MatchMode& mode);
        bool match(const std::wstring& string, const std::optional<MatchMode>& matchMode = {}) const;
        /**
         * @brief Checks the format string of this trigger.
         * @throws clip::ClipboardTriggerFormatException Throws when the format is invalid.
         */
        void checkFormat() const;
        std::wstring formatTrigger(const std::wstring& stringView);

        bool operator==(ClipboardTrigger& other);

    private:
        clip::utils::Logger logger{ L"ClipboardTrigger" };
        std::wstring _label{};
        std::wstring _format{};
        boost::wregex _regex{};
        bool _enabled = true;
        std::optional<MatchMode> _matchMode{};
        bool _useRegexMatchResults = true;

        ClipboardTrigger(boost::property_tree::wptree& triggersNode);

        std::optional<ClipboardTriggerFormatException> checkFormat(const std::wstring& format) const;
        boost::property_tree::wptree serialize() const;
    };
}

