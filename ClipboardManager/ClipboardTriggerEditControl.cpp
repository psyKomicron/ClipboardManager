#include "pch.h"
#include "ClipboardTriggerEditControl.h"
#if __has_include("ClipboardTriggerEditControl.g.cpp")
#include "ClipboardTriggerEditControl.g.cpp"
#endif

#include "src/res/strings.h"

namespace xaml
{
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Controls;
}

namespace winrt::ClipboardManager::implementation
{
    ClipboardTriggerEditControl::ClipboardTriggerEditControl()
    {
        visualStateManager.initializeStates(
            {
                regexErrorState,
                regexNoErrorState,
                labelErrorState,
                labelNoErrorState,
                formatErrorState,
                formatNoErrorState,
            });
    }

    winrt::hstring ClipboardTriggerEditControl::TriggerLabel() const
    {
        return _triggerLabel.get();
    }

    void ClipboardTriggerEditControl::TriggerLabel(const winrt::hstring& value)
    {
        _triggerLabel.set(value);
    }

    winrt::hstring ClipboardTriggerEditControl::TriggerFormat() const
    {
        return _triggerFormat.get();
    }

    void ClipboardTriggerEditControl::TriggerFormat(const winrt::hstring& value)
    {
        _triggerFormat.set(value);
    }

    winrt::hstring ClipboardTriggerEditControl::TriggerRegex() const
    {
        return _triggerRegex.get();
    }

    void ClipboardTriggerEditControl::TriggerRegex(const winrt::hstring& value)
    {
        _triggerRegex.set(value);
    }

    bool ClipboardTriggerEditControl::IgnoreCase() const
    {
        return _ignoreCase.get();
    }

    void ClipboardTriggerEditControl::IgnoreCase(const bool& value)
    {
        _ignoreCase.set(value);
    }

    bool ClipboardTriggerEditControl::UseSearch() const
    {
        return _useSearch.get();
    }

    void ClipboardTriggerEditControl::UseSearch(const bool& value)
    {
        _useSearch.set(value);
    }


    winrt::Windows::Foundation::IAsyncOperation<bool> ClipboardTriggerEditControl::Edit()
    {
        auto result = co_await EditDialog().ShowAsync();

        co_return result == xaml::ContentDialogResult::Primary;
    }


    void ClipboardTriggerEditControl::EditDialog_Opened(xaml::ContentDialog const& sender, xaml::ContentDialogOpenedEventArgs const& args)
    {
        CheckInput();
    }

    void ClipboardTriggerEditControl::TextBox_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::TextChangedEventArgs const& e)
    {
        CheckInput();
    }

    winrt::Windows::Foundation::IInspectable ClipboardTriggerEditControl::asInspectable()
    {
        return *this;
    }


    void ClipboardTriggerEditControl::CheckInput()
    {
        auto checkFormat = CheckFormat();
        auto checkRegex = CheckRegex();
        visualStateManager.goToState(LabelTextBox().Text().empty() ? labelErrorState : labelNoErrorState);
        visualStateManager.goToState(!checkFormat ? formatErrorState : formatNoErrorState);
        visualStateManager.goToState(!checkRegex ? regexErrorState : regexNoErrorState);

        EditDialog().IsPrimaryButtonEnabled(
            !LabelTextBox().Text().empty()
            && checkRegex
            && checkFormat
        );
    }

    bool ClipboardTriggerEditControl::CheckFormat()
    {
        bool valid = false;
        winrt::hstring message{};
        
        if (!FormatTextBox().Text().empty())
        {
            auto text = std::wstring(FormatTextBox().Text());
            auto missingOpener = text.find(L'{') == std::wstring::npos;
            auto missingCloser = text.find(L'}') == std::wstring::npos;

            if (missingOpener && missingCloser)
            { 
                message = resLoader.getOrAlt(L"StringFormatError_MissingBraces", clip::res::StringFormatError_MissingBraces);
            }
            else if (missingOpener)
            {
                message = resLoader.getOrAlt(L"StringFormatError_MissingOpener", clip::res::StringFormatError_MissingOpener);
            }
            else
            {
                message = resLoader.getOrAlt(L"StringFormatError_MissingCloser", clip::res::StringFormatError_MissingCloser);
            }

            valid = !(missingOpener || missingCloser);

            visualStateManager.goToState((text.starts_with(L"https://") || text.starts_with(L"http://")) 
                ? formatNoWarningState 
                : formatWarningState);
        }
        else
        {
            message = resLoader.getOrAlt(L"StringFormatError_Empty", clip::res::StringFormatError_Empty);
        }

        FormatTextBoxError().Text(message);

        return valid;
    }

    bool ClipboardTriggerEditControl::CheckRegex()
    {
        auto text = std::wstring(RegexTextBox().Text().data());
        hstring message{};

        if (!text.empty())
        {
            try
            {
                // Simply build a boost::regex to see if the text input is valid.
                // Catch block informs the user about the invalid data.
                boost::wregex re{ text };
                return true;
            }
            catch (boost::regex_error regexError)
            {
                auto what = regexError.what();
                logger.error(what);
                logger.error(std::to_wstring(regexError.code()));

                auto position = regexError.position();
                message = std::vformat(resLoader.getOrAlt(L"StringRegexError_Invalid", clip::res::StringRegexError_Invalid), std::make_wformat_args(position));
            }
        }
        else
        {
            message = resLoader.getOrAlt(L"StringRegexError_EmptyString", clip::res::StringRegexError_EmptyString);
        }

        RegexTextBoxError().Text(message);

        return false;
    }
}
