#include "pch.h"
#include "RegexTesterControl.h"
#if __has_include("RegexTesterControl.g.cpp")
#include "RegexTesterControl.g.cpp"
#endif

#include <src/utils/helpers.hpp>

#include <boost/regex.hpp>

#include <winrt/Microsoft.UI.Xaml.Documents.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>

namespace ui
{
    using namespace winrt::Microsoft::UI;
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Controls;
    using namespace winrt::Microsoft::UI::Xaml::Documents;
    using namespace winrt::Microsoft::UI::Xaml::Media;
}

namespace winrt::ClipboardManager::implementation
{
    winrt::async RegexTesterControl::Open()
    {
        co_await TestRegexContentDialog().ShowAsync();
    }

    void RegexTesterControl::refreshMatches()
    {
        std::wstring regexText = RegexTextBox().Text().data();
        std::wstring input = TestInputTextBox().Text().data();
        std::wstring format = FormatTextBox().Text().data();
        if (format.empty())
        {
            format = L"{}";
        }

        MatchTextBlockParagraph().Inlines().Clear();
        MatchTextBlock().TextHighlighters().Clear();
        visualStateManager.goToState(noErrorState);

        if (!regexText.empty())
        {
            try
            {
                boost::wregex::flag_type flags = RegexIgnoreCaseToggleButton().IsChecked().GetBoolean() ? boost::regex_constants::icase : boost::regex_constants::normal;
                bool useRegexMatchResults = UseRegexMatchResultsToggleButton().IsChecked().GetBoolean();
                boost::wregex regex{ regexText, flags };

                /*boost::wsregex_token_iterator iterator{ input.begin(), input.end(), regex, 0 };
                boost::wsregex_token_iterator end{};

                for (; iterator != end; iterator++)
                {
                    std::wcout << L"Token: " << *iterator << std::endl;
                }*/

                if (!input.empty())
                {
                    boost::wsmatch matches{};
                    bool result = RegexModeToggleButton().IsChecked().GetBoolean()
                        ? boost::regex_search(input, matches, regex)
                        : boost::regex_match(input, matches, regex);

                    if (result)
                    {
                        ui::Run run{};
                        MatchTextBlockParagraph().Inlines().Append(run);

                        ui::SolidColorBrush backgroundBrush{};
                        backgroundBrush.Color(ui::Colors::LightGreen());
                        backgroundBrush.Opacity(0.3);
                        ui::TextHighlighter highlighter{};
                        highlighter.Background(backgroundBrush);

                        std::wstring url = L"";
                        if (useRegexMatchResults && matches.size() > 1)
                        {
                            auto matchString = matches[1].str();
                            url = std::vformat(format, std::make_wformat_args(matchString));

                            regex = boost::wregex(matchString);
                            assert(boost::regex_search(url, matches, regex) /*Regex search should work as we just inserted the text we are searching.*/);
                        }
                        else
                        {
                            url = std::vformat(format, std::make_wformat_args(input));
                        }

                        run.Text(url);

                        for (size_t i = 0; i < matches.size(); i++)
                        {
                            auto str = matches[i].str();
                            size_t pos = url.find(str);
                            size_t length = str.size();

                            highlighter.Ranges().Append(ui::TextRange(pos, length));
                        }

                        MatchTextBlock().TextHighlighters().Append(highlighter);
                    }
                }
            }
            catch (boost::regex_error regexError)
            {
                visualStateManager.goToState(errorState);
                ErrorGrid().Message(clip::utils::to_wstring(regexError.what()));
            }
            catch (...)
            {
                logger.error(L"Unknown error while refreshing matches.");
            }
        }
    }


    void RegexTesterControl::TestRegexContentDialog_CloseButtonClick(winrt::Windows::Foundation::IInspectable const& sender, ui::RoutedEventArgs const& e)
    {
        TestRegexContentDialog().Hide();
    }

    void RegexTesterControl::RegexTextBox_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, ui::TextChangedEventArgs const& e)
    {
        refreshMatches();
    }

    void RegexTesterControl::TestInputTextBox_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, ui::TextChangedEventArgs const& e)
    {
        refreshMatches();
    }

    void RegexTesterControl::ToggleButton_Click(winrt::Windows::Foundation::IInspectable const& sender, ui::RoutedEventArgs const& e)
    {
        refreshMatches();
    }

    void RegexTesterControl::FormatTextBox_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, ui::TextChangedEventArgs const& e)
    {
        refreshMatches();
    }
}
