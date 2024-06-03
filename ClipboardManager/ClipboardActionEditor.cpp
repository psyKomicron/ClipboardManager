#include "pch.h"
#include "ClipboardActionEditor.h"
#if __has_include("ClipboardActionEditor.g.cpp")
#include "ClipboardActionEditor.g.cpp"
#endif

#include <src/utils/helpers.hpp>

#include <boost/regex.hpp>

#include <iostream>

namespace impl = winrt::ClipboardManager::implementation;
namespace winrt
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Controls;
}

impl::ClipboardActionEditor::ClipboardActionEditor()
{
    visualStateManager.initializeStates(
    {
        EnabledState,
        DisabledState
    });
}

winrt::hstring impl::ClipboardActionEditor::ActionLabel() const
{
    return _actionLabel;
}

void impl::ClipboardActionEditor::ActionLabel(const winrt::hstring& value)
{
    _actionLabel = value;
    NotifyPropertyChanged();
}

winrt::hstring impl::ClipboardActionEditor::ActionFormat() const
{
    return _actionFormat;
}

void impl::ClipboardActionEditor::ActionFormat(const winrt::hstring& value)
{
    _actionFormat = value;
    NotifyPropertyChanged();
}

winrt::hstring impl::ClipboardActionEditor::ActionRegex() const
{
    return _actionRegex;
}

void impl::ClipboardActionEditor::ActionRegex(const winrt::hstring& value)
{
    _actionRegex = value;
    NotifyPropertyChanged();
}

bool winrt::ClipboardManager::implementation::ClipboardActionEditor::ActionEnabled() const
{
    return _actionEnabled;
}

void winrt::ClipboardManager::implementation::ClipboardActionEditor::ActionEnabled(const bool& value)
{
    _actionEnabled = value;
}

winrt::event_token winrt::ClipboardManager::implementation::ClipboardActionEditor::IsOn(const event_is_on_t& handler)
{
    return e_isOn.add(handler);
}

void winrt::ClipboardManager::implementation::ClipboardActionEditor::IsOn(const winrt::event_token& token)
{
    e_isOn.remove(token);
}

winrt::event_token winrt::ClipboardManager::implementation::ClipboardActionEditor::LabelChanged(const event_changed_t& handler)
{
    return e_labelChanged.add(handler);
}

void winrt::ClipboardManager::implementation::ClipboardActionEditor::LabelChanged(const winrt::event_token& token)
{
    e_labelChanged.remove(token);
}

winrt::event_token winrt::ClipboardManager::implementation::ClipboardActionEditor::FormatChanged(const event_changed_t& handler)
{
    return e_formatChanged.add(handler);
}

void winrt::ClipboardManager::implementation::ClipboardActionEditor::FormatChanged(const winrt::event_token& token)
{
    e_formatChanged.remove(token);
}

void impl::ClipboardActionEditor::RemoveButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    // TODO: Add event to notify parent control that this editor need to be removed.
}

winrt::async impl::ClipboardActionEditor::EditButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    if ((co_await EditDialog().ShowAsync()) == winrt::ContentDialogResult::Primary)
    {
        // TODO: Add event if the user has edited (with new changes) this action.
        auto newFormat = FormatTextBox().Text();
        auto newRegex = RegexTextBox().Text();
        auto newLabel = LabelTextBox().Text();

        if (newFormat != _actionFormat)
        {
            auto oldFormat = _actionFormat;
            ActionFormat(newFormat);
            e_formatChanged(*this, oldFormat);
        }

        if (newRegex != _actionRegex)
        {
            auto oldRegex = _actionRegex;
            ActionRegex(newRegex);
            // TODO: Update parent.
        }

        if (newLabel != _actionLabel)
        {
            auto oldLabel = _actionLabel;
            ActionLabel(newLabel);
            e_labelChanged(*this, oldLabel);
        }
    }
}

void impl::ClipboardActionEditor::ActionEnabledToggleSwitch_Toggled(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    check_loaded(loaded);
    // TODO: Add event to notify parent control that this action has been disabled.
    visualStateManager.switchState(EnabledState.group());
}

void impl::ClipboardActionEditor::UserControl_Loaded(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    loaded = true;
}


void impl::ClipboardActionEditor::NotifyPropertyChanged(std::source_location sourceLocation)
{
    e_propertyChanged(*this, clipmgr::utils::PropChangedEventArgs::create(sourceLocation));
}

