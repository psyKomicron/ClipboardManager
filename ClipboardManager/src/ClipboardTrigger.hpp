#pragma once
#include <boost/regex.hpp>
#include <boost/property_tree/ptree.hpp>

#include <vector>
#include <filesystem>

namespace clipmgr
{
    enum class MatchMode
    {
        Match,
        Search
    };

    class ClipboardTrigger
    {
    public:
        ClipboardTrigger(const std::wstring& label, const std::wstring& format, const boost::wregex& regex, const bool& enabled);

        static std::vector<ClipboardTrigger> loadClipboardActions(const std::filesystem::path& userFilePath);
        static void initializeSaveFile(const std::filesystem::path& userFilePath);

        std::wstring label() const;
        void label(const std::wstring& value);
        std::wstring format() const;
        void format(const std::wstring& value);
        boost::wregex regex() const;
        void regex(const boost::wregex& regex);
        bool enabled() const;
        void enabled(const bool& value);

        void updateMatchMode(const MatchMode& mode);
        bool match(const std::wstring& string) const;

        bool operator==(ClipboardTrigger& other);
        //bool operator==(ClipboardTrigger& other);

    private:
        std::wstring _label{};
        std::wstring _format{};
        boost::wregex _regex{};
        bool _enabled = true;
        MatchMode _matchMode = MatchMode::Match;

        static void firstTimeInitialization(const std::filesystem::path& path, boost::property_tree::wptree tree);
        static boost::wregex parseRegexFromXml(boost::property_tree::wptree& options);
    };
}

