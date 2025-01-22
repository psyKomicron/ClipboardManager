#pragma once
#include <boost/property_tree/ptree.hpp>

#include <vector>
#include <string>

namespace clip
{
    constexpr auto TIME_T_FORMAT_DES = L"%F %R";
    constexpr auto TIME_T_FORMAT_SER = L"{0:%F} {0:%R}";
}

namespace clip
{
    /**
     * @brief Model class of ClipboardActionView. Handles serialization.
     */
    class ClipboardAction
    {
    public:
        using time_t = std::chrono::time_point<std::chrono::system_clock>;

        /**
         * @brief Deserialize a clipboard action.
         * @param node Data
         */
        ClipboardAction(boost::property_tree::wptree& node);
        /**
         * @brief Creates a clipboard action with std::system_clock::now() as it's creation time.
         */
        ClipboardAction(const std::wstring_view&& text);
        /**
         * @brief Creates a clipboard action with the text and DateTime.
         */
        ClipboardAction(const std::wstring_view&& text, const winrt::Windows::Foundation::DateTime& timestamp);

        std::wstring text() const;
        time_t creationTime() const;

        void save(boost::property_tree::wptree& tree);

    private:
        std::wstring _text{};
        time_t _creationTime{};
    };
}

//namespace std
//{
//    template<>
//    class std::formatter<clip::ClipboardAction> : std::formatter<std::wstring>
//    {
//        constexpr auto parse(std::wformat_parse_context& ctx)
//        {
//            return ctx.begin();
//        }
//
//        auto format(const clip::ClipboardAction& obj, std::wformat_context& ctx) const
//        {
//            std::wstring temp{};
//            std::format_to(std::back_inserter(temp), "action\\{text: {}, creationTime: {1:%F} {1:%R}\\}", obj.text(), obj.creationTime());
//
//            return std::formatter<std::wstring>::format(temp, ctx);
//        }
//    };
//}
