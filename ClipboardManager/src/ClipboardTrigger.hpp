#pragma once
#include <src/utils/Logger.hpp>

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

    enum class FormatExceptionCode : uint32_t
    {
        Unknown = 0,
        MissingOpenBraces = 1,
        MissingClosingBraces = 1 << 1,
        InvalidFormat = 1 << 2
    };

    inline FormatExceptionCode operator|(const FormatExceptionCode& left, const FormatExceptionCode& right)
    {
        return static_cast<FormatExceptionCode>(static_cast<int32_t>(left) | static_cast<int32_t>(right));
    }

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
        static void initializeSaveFile(const std::filesystem::path& userFilePath);

        std::wstring label() const;
        void label(const std::wstring& value);
        std::wstring format() const;
        void format(const std::wstring& value);
        boost::wregex regex() const;
        void regex(const boost::wregex& regex);
        bool enabled() const;
        void enabled(const bool& value);
        std::optional<clip::MatchMode> matchMode() const;

        void updateMatchMode(const MatchMode& mode);
        bool match(const std::wstring& string, const std::optional<MatchMode>& matchMode = {}) const;
        /**
         * @brief Checks the format string of this trigger.
         * @throws clip::ClipboardTriggerFormatException Throws when the format is invalid.
         */
        void checkFormat();

        bool operator==(ClipboardTrigger& other);

    private:
        std::wstring _label{};
        std::wstring _format{};
        boost::wregex _regex{};
        bool _enabled = true;
        std::optional<MatchMode> _matchMode{};

        static void firstTimeInitialization(const std::filesystem::path& path, boost::property_tree::wptree tree);
        //static boost::wregex parseRegexFromXml(boost::property_tree::wptree& options);
        ClipboardTriggerFormatException checkFormat(const std::wstring& format);
    };
}

