#include "pch.h"
#include "RegexTesterControl.h"
#if __has_include("RegexTesterControl.g.cpp")
#include "RegexTesterControl.g.cpp"
#endif

#ifdef _DEBUG
#include <iostream>
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

    void RegexTesterControl::TestRegexContentDialog_CloseButtonClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        TestRegexContentDialog().Hide();
    }

    void RegexTesterControl::RegexTextBox_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::TextChangedEventArgs const& e)
    {
        refreshMatches();
    }

    void RegexTesterControl::TestInputTextBox_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::TextChangedEventArgs const& e)
    {
        refreshMatches();
    }

    void RegexTesterControl::ToggleButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        refreshMatches();
    }


    void RegexTesterControl::refreshMatches()
    {
        std::wstring regexText = RegexTextBox().Text().data();
        std::wstring input = TestInputTextBox().Text().data();

        MatchTextBlockParagraph().Inlines().Clear();
        MatchTextBlock().TextHighlighters().Clear();
        visualStateManager.goToState(noErrorState);

        if (!regexText.empty() && !input.empty())
        {
            ui::Run run{};
            run.Text(input);
            MatchTextBlockParagraph().Inlines().Append(run);

            ui::TextHighlighter highlighter{};
            ui::SolidColorBrush backgroundBrush{};
            backgroundBrush.Color(ui::Colors::LightGreen());
            backgroundBrush.Opacity(0.5);
            highlighter.Background(backgroundBrush);

            try
            {
                boost::wregex::flag_type flags = RegexIgnoreCaseToggleButton().IsChecked().GetBoolean() ? boost::regex_constants::icase : boost::regex_constants::normal;
                boost::wregex regex{ regexText, flags };
                boost::wsmatch match{};

                bool result = false;

                /*boost::wsregex_token_iterator iterator{ input.begin(), input.end(), regex, 0 };
                boost::wsregex_token_iterator end{};

                for (; iterator != end; iterator++)
                {
                    std::wcout << L"Token: " << *iterator << std::endl;
                }*/

                if (RegexModeToggleButton().IsChecked().GetBoolean())
                {
                    result = boost::regex_search(input, match, regex);
                }
                else
                {
                    result = boost::regex_match(input, match, regex);
                }

                if (result)
                {
                    logger.info(std::to_wstring(match.size()) + L" matches");

                    for (size_t i = 0; i < match.size(); i++)
                    {
                        auto str = match[i].str();
                        size_t pos = input.find(str);
                        size_t length = str.size();

                        ui::TextRange range
                        {
                            pos,
                            length
                        };

                        highlighter.Ranges().Append(range);

                        logger.debug(L"Match number " + std::to_wstring(i) + L" -- " + str);
                    }

                    MatchTextBlock().TextHighlighters().Append(highlighter);
                }
            }
            catch (boost::regex_error regexError)
            {
                visualStateManager.goToState(errorState);
                ErrorGrid().Message(clip::utils::convert(regexError.what()));

                /*uint32_t index = 0;
                if (MatchTextBlockParagraph().Inlines().IndexOf(run, index))
                {
                    MatchTextBlockParagraph().Inlines().RemoveAt(index);
                }*/
            }
        }
    }
}
