#include "pch.h"
#include "ClipboardActionEditor.h"
#if __has_include("ClipboardActionEditor.g.cpp")
#include "ClipboardActionEditor.g.cpp"
#endif

#include <src/ClipboardTrigger.hpp>
#include <src/utils/helpers.hpp>
#include <src/res/strings.h>

#include <boost/regex.hpp>

#include <pplawait.h>
#include <iostream>

namespace winrt
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Controls;
}
namespace xaml
{
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Media;
}


namespace winrt::ClipboardManager::implementation
{
    ClipboardActionEditor::ClipboardActionEditor()
    {
        visualStateManager.initializeStates(
            {
                EnabledState,
                DisabledState,
                NoLabelErrorState,
                LabelErrorState,
                NoRegexErrorState,
                RegexErrorState,
                NoFormatErrorState,
                FormatErrorState
            });
    }

    winrt::hstring ClipboardActionEditor::ActionLabel() const
    {
        return _triggerLabelProperty.get();
    }

    void ClipboardActionEditor::ActionLabel(const winrt::hstring& value)
    {
        _triggerLabelProperty.set(value);
    }

    winrt::hstring ClipboardActionEditor::ActionFormat() const
    {
        return _triggerFormatProperty.get();
    }

    void ClipboardActionEditor::ActionFormat(const winrt::hstring& value)
    {
        _triggerFormatProperty.set(value);
    }

    winrt::hstring ClipboardActionEditor::ActionRegex() const
    {
        return _triggerRegexProperty.get();
    }

    void ClipboardActionEditor::ActionRegex(const winrt::hstring& value)
    {
        _triggerRegexProperty.set(value);
    }

    bool ClipboardActionEditor::ActionEnabled() const
    {
        return _actionEnabledProperty.get();
    }

    void ClipboardActionEditor::ActionEnabled(const bool& value)
    {
        _actionEnabledProperty.set(value);

        visualStateManager.goToEnabledState(_actionEnabledProperty.get());
        e_isOn(*this, _actionEnabledProperty.get());
    }

    bool implementation::ClipboardActionEditor::IgnoreCase() const
    {
        return _ignoreCaseProperty.get();
    }

    void implementation::ClipboardActionEditor::IgnoreCase(const bool& value)
    {
        _ignoreCaseProperty.set(value);
    }

    bool implementation::ClipboardActionEditor::UseSearch() const
    {
        return _useSearchProperty.get();
    }

    void implementation::ClipboardActionEditor::UseSearch(const bool& value)
    {
        _useSearchProperty.set(value);
    }


    winrt::event_token ClipboardActionEditor::IsOn(const event_is_on_t& handler)
    {
        return e_isOn.add(handler);
    }

    void ClipboardActionEditor::IsOn(const winrt::event_token& token)
    {
        e_isOn.remove(token);
    }

    winrt::event_token ClipboardActionEditor::LabelChanged(const event_label_changed_t& handler)
    {
        return e_labelChanged.add(handler);
    }

    void ClipboardActionEditor::LabelChanged(const winrt::event_token& token)
    {
        e_labelChanged.remove(token);
    }

    winrt::event_token ClipboardActionEditor::Changed(const event_changed_t& handler)
    {
        return e_changed.add(handler);
    }

    void ClipboardActionEditor::Changed(const winrt::event_token& token)
    {
        e_changed.remove(token);
    }

    winrt::event_token ClipboardActionEditor::Removed(const event_changed_t& handler)
    {
        return e_removed.add(handler);
    }

    void ClipboardActionEditor::Removed(const winrt::event_token& token)
    {
        e_removed.remove(token);
    }


    winrt::async ClipboardActionEditor::StartTour()
    {
        ClipboardActionRootGridTeachingTip().IsOpen(true);

        co_await concurrency::create_task([this]()
        {
            this->waitFlag.wait(false);
            this->waitFlag.clear();
        });
    }

    winrt::Windows::Foundation::IAsyncOperation<bool> ClipboardActionEditor::Edit()
    {
        TriggerEditControl().TriggerFormat(_triggerFormatProperty.get());
        TriggerEditControl().TriggerLabel(_triggerLabelProperty.get());
        TriggerEditControl().TriggerRegex(_triggerRegexProperty.get());
        TriggerEditControl().IgnoreCase(_ignoreCaseProperty.get());
        TriggerEditControl().UseSearch(_useSearchProperty.get());

        auto result = co_await TriggerEditControl().Edit();
        if (result)
        {
            winrt::hstring newFormat = TriggerEditControl().TriggerFormat();
            winrt::hstring newLabel = TriggerEditControl().TriggerLabel();
            winrt::hstring newRegex = TriggerEditControl().TriggerRegex();

            // Update new format.
            ActionFormat(newFormat);

            // Update regex.
            ActionRegex(newRegex);

            // Update matching mode and regex flags.
            _ignoreCaseProperty.set(TriggerEditControl().IgnoreCase());
            _useSearchProperty.set(TriggerEditControl().UseSearch());

            auto flags = (static_cast<int32_t>(_ignoreCaseProperty.get()) << 1) | static_cast<int32_t>(_useSearchProperty.get());

            e_changed(*this, box_value(flags));

            // Update label.
            winrt::hstring oldLabel = _triggerLabelProperty.get();
            if (oldLabel != newLabel)
            {
                ActionLabel(newLabel);

                e_labelChanged(*this, oldLabel);
            }
        }

        co_return result;
    }


    void ClipboardActionEditor::UserControl_Loaded(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
    {
        loaded = true;

        visualStateManager.goToEnabledState(_actionEnabledProperty.get());

        // Check if the associated trigger is valid.
        auto trigger = clip::ClipboardTrigger(
            std::wstring(_triggerLabelProperty.get()), 
            std::wstring(_triggerFormatProperty.get()), 
            boost::wregex(std::wstring(_triggerRegexProperty.get())), true);

        try
        {
            trigger.checkFormat();
        }
        catch (clip::ClipboardTriggerFormatException formatException)
        {
            hstring message{};
            switch (formatException.code())
            {
                case clip::FormatExceptionCode::MissingOpenBraces:
                    message = resLoader.getOrAlt(L"StringFormatError_MissingOpener", clip::res::StringFormatError_MissingOpener);
                    break;
                case clip::FormatExceptionCode::MissingClosingBraces:
                    message = resLoader.getOrAlt(L"StringFormatError_MissingCloser", clip::res::StringFormatError_MissingCloser);
                    break;
                case clip::FormatExceptionCode::EmptyString:
                    message = resLoader.getOrAlt(L"StringFormatError_Empty", clip::res::StringFormatError_Empty);
                    break;
                case clip::FormatExceptionCode::ArgumentNotFound:
                {
                    message = resLoader.getOrAlt(L"StringFormatError_ArgumentNotFound", clip::res::StringFormatError_ArgumentNotFound);
                    auto argument = formatException.message();
                    message = std::vformat(message, std::make_wformat_args(argument));
                    break;
                }
                case (clip::FormatExceptionCode::MissingOpenBraces | clip::FormatExceptionCode::MissingClosingBraces):
                    message = resLoader.getOrAlt(L"StringFormatError_MissingBraces", clip::res::StringFormatError_MissingBraces);
                    break;
                case clip::FormatExceptionCode::InvalidFormat:
                case clip::FormatExceptionCode::Unknown:
                default:
                    message = resLoader.getOrAlt(L"StringFormatError_InvalidFormat", L"Invalid format string");
                    break;
            }

            switch (formatException.code())
            {
                // Warning errors
                case clip::FormatExceptionCode::EmptyString:
                case (clip::FormatExceptionCode::MissingOpenBraces | clip::FormatExceptionCode::MissingClosingBraces):
                    visualStateManager.goToState(FormatWarningState);
                    break;

                // Errors
                case clip::FormatExceptionCode::MissingOpenBraces:
                case clip::FormatExceptionCode::MissingClosingBraces:
                case clip::FormatExceptionCode::ArgumentNotFound:
                case clip::FormatExceptionCode::InvalidFormat:
                case clip::FormatExceptionCode::Unknown:
                default:
                    visualStateManager.goToState(FormatErrorState);
                    break;
            }

            FormatErrorTextBlock().Text(message);
        }
    }

    void ClipboardActionEditor::RemoveButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
    {
        e_removed(*this, nullptr);
    }

    winrt::async ClipboardActionEditor::EditButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
    {
        co_await Edit();
    }

    void ClipboardActionEditor::ActionEnabledToggleSwitch_Toggled(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
    {
        check_loaded(loaded);
    }

    void ClipboardActionEditor::RootGrid_PointerEntered(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e)
    {
        visualStateManager.goToState(clip::ui::VisualState<ClipboardActionEditor>(L"PointerOver", 1, false));
    }

    void ClipboardActionEditor::RootGrid_PointerExited(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e)
    {
        visualStateManager.goToState(clip::ui::VisualState<ClipboardActionEditor>(L"PointerOut", 1, false));
    }

    void ClipboardActionEditor::TeachingTip_CloseButtonClick(winrt::TeachingTip const& sender, winrt::IInspectable const& args)
    {
        static size_t index = 1;
        static std::vector<winrt::TeachingTip> teachingTips
        {
            ClipboardActionRootGridTeachingTip(),
            ActionLabelTextBlockTeachingTip(),
            ActionFormatTextBlockTeachingTip(),
            ActionRegexTextBlockTeachingTip(),
            ActionEnabledToggleSwitchTeachingTip(),
            RemoveButtonTeachingTip(),
            EditButtonTeachingTip()
        };

        if (index < teachingTips.size())
        {
            teachingTips[index - 1].IsOpen(false);
            teachingTips[index++].IsOpen(true);
        }
        else
        {
            teachingTips[index - 1].IsOpen(false);
            waitFlag.test_and_set();
            waitFlag.notify_all();
        }
    }


    void ClipboardActionEditor::useSearchPropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs args)
    {
        visualStateManager.goToState(_useSearchProperty.get() ? UseSearchEnabledState : UseSearchDisabledState);
        raisePropertyChanged(args);
    }

    void ClipboardActionEditor::ignoreCasePropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs args)
    {
        visualStateManager.goToState(_ignoreCaseProperty.get() ? IgnoreCaseEnabledState : IgnoreCaseDisabledState);
        raisePropertyChanged(args);
    }

    winrt::Windows::Foundation::IInspectable ClipboardActionEditor::asInspectable()
    {
        return *this;
    }
}


