#include "pch.h"
#include "ClipboardActionEditor.h"
#if __has_include("ClipboardActionEditor.g.cpp")
#include "ClipboardActionEditor.g.cpp"
#endif

#include <src/utils/helpers.hpp>

#include <winrt/Windows.Foundation.Collections.h>

#include <boost/regex.hpp>

#include <pplawait.h>
#include <iostream>

namespace impl = winrt::ClipboardManager::implementation;
namespace implementation = winrt::ClipboardManager::implementation;
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

impl::ClipboardActionEditor::ClipboardActionEditor()
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

winrt::hstring impl::ClipboardActionEditor::ActionLabel() const
{
    return _actionLabelProperty.get();
}

void impl::ClipboardActionEditor::ActionLabel(const winrt::hstring& value)
{
    _actionLabelProperty.set(value);
}

winrt::hstring impl::ClipboardActionEditor::ActionFormat() const
{
    return _actionFormatProperty.get();
}

void impl::ClipboardActionEditor::ActionFormat(const winrt::hstring& value)
{
    _actionFormatProperty.set(value);
}

winrt::hstring impl::ClipboardActionEditor::ActionRegex() const
{
    return _actionRegexProperty.get();
}

void impl::ClipboardActionEditor::ActionRegex(const winrt::hstring& value)
{
    _actionRegexProperty.set(value);
}

bool impl::ClipboardActionEditor::ActionEnabled() const
{
    return _actionEnabledProperty.get();
}

void impl::ClipboardActionEditor::ActionEnabled(const bool& value)
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


winrt::event_token impl::ClipboardActionEditor::IsOn(const event_is_on_t& handler)
{
    return e_isOn.add(handler);
}

void impl::ClipboardActionEditor::IsOn(const winrt::event_token& token)
{
    e_isOn.remove(token);
}

winrt::event_token impl::ClipboardActionEditor::LabelChanged(const event_changed_t& handler)
{
    return e_labelChanged.add(handler);
}

void impl::ClipboardActionEditor::LabelChanged(const winrt::event_token& token)
{
    e_labelChanged.remove(token);
}

winrt::event_token impl::ClipboardActionEditor::FormatChanged(const event_changed_t& handler)
{
    return e_formatChanged.add(handler);
}

void impl::ClipboardActionEditor::FormatChanged(const winrt::event_token& token)
{
    e_formatChanged.remove(token);
}

winrt::event_token winrt::ClipboardManager::implementation::ClipboardActionEditor::RegexChanged(const event_re_changed_t& handler)
{
    return e_regexChanged.add(handler);
}

void winrt::ClipboardManager::implementation::ClipboardActionEditor::RegexChanged(const winrt::event_token& token)
{
    e_regexChanged.remove(token);
}

winrt::event_token impl::ClipboardActionEditor::Removed(const event_removed_t& handler)
{
    return e_removed.add(handler);
}

void impl::ClipboardActionEditor::Removed(const winrt::event_token& token)
{
    e_removed.remove(token);
}


winrt::async impl::ClipboardActionEditor::StartTour()
{
    ClipboardActionRootGridTeachingTip().IsOpen(true);

    co_await concurrency::create_task([this]()
    {
        this->waitFlag.wait(false);
        this->waitFlag.clear();
    });
}

winrt::Windows::Foundation::IAsyncOperation<bool> winrt::ClipboardManager::implementation::ClipboardActionEditor::Edit()
{
    CheckInput();

    auto result = co_await EditDialog().ShowAsync();
    if (result == winrt::ContentDialogResult::Primary)
    {
        winrt::hstring newFormat = FormatTextBox().Text();
        winrt::hstring newLabel = LabelTextBox().Text();
        winrt::hstring newRegex = RegexTextBox().Text();

        winrt::hstring oldFormat = _actionFormatProperty.get();
        ActionFormat(newFormat);
        e_formatChanged(*this, oldFormat);

        winrt::hstring oldRegex = _actionRegexProperty.get();
        ActionRegex(newRegex);
        auto flags = (static_cast<int32_t>(_ignoreCaseProperty.get()) << 1) | static_cast<int32_t>(_useSearchProperty.get());
        e_regexChanged(*this, box_value(flags));

        winrt::hstring oldLabel = _actionLabelProperty.get();
        ActionLabel(newLabel);
        e_labelChanged(*this, oldLabel);
    }

    co_return result == winrt::ContentDialogResult::Primary;
}


void impl::ClipboardActionEditor::UserControl_Loaded(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    loaded = true;
    visualStateManager.goToEnabledState(_actionEnabledProperty.get());
}

void impl::ClipboardActionEditor::RemoveButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    e_removed(*this, nullptr);
}

winrt::async impl::ClipboardActionEditor::EditButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    co_await Edit();
}

void impl::ClipboardActionEditor::ActionEnabledToggleSwitch_Toggled(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    check_loaded(loaded);

    
}

void impl::ClipboardActionEditor::TeachingTip_CloseButtonClick(winrt::TeachingTip const& sender, winrt::IInspectable const& args)
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

void implementation::ClipboardActionEditor::LabelTextBox_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::TextChangedEventArgs const& e)
{
    CheckInput();

    if (LabelTextBox().Text().empty())
    {
        visualStateManager.goToState(LabelErrorState);
    }
    else
    {
        visualStateManager.goToState(NoLabelErrorState);
    }
}

void implementation::ClipboardActionEditor::FormatTextBox_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::TextChangedEventArgs const& e)
{
    CheckInput();

    if (FormatTextBox().Text().empty())
    {
        visualStateManager.goToState(FormatErrorState);
    }
    else
    {
        visualStateManager.goToState(NoFormatErrorState);
    }
}

void implementation::ClipboardActionEditor::RegexTextBox_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::TextChangedEventArgs const& e)
{
    CheckInput();

    if (RegexTextBox().Text().empty())
    {
        visualStateManager.goToState(RegexErrorState);
    }
    else
    {
        visualStateManager.goToState(NoRegexErrorState);
    }
}


void implementation::ClipboardActionEditor::CheckInput()
{
    EditDialog().IsPrimaryButtonEnabled(
        !RegexTextBox().Text().empty()
        && !FormatTextBox().Text().empty()
        && !LabelTextBox().Text().empty()
    );
}

winrt::Windows::Foundation::IInspectable winrt::ClipboardManager::implementation::ClipboardActionEditor::asInspectable()
{
    return *this;
}
