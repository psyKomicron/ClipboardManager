#include "pch.h"
#include "ClipboardActionEditor.h"
#if __has_include("ClipboardActionEditor.g.cpp")
#include "ClipboardActionEditor.g.cpp"
#endif

#include <src/utils/helpers.hpp>

namespace impl = winrt::ClipboardManager::implementation;
namespace winrt
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Microsoft::UI::Xaml;
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

void impl::ClipboardActionEditor::RemoveButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    // TODO: Add event to notify parent control that this editor need to be removed.
}

winrt::async impl::ClipboardActionEditor::EditButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    co_await EditDialog().ShowAsync();
    // TODO: Add event if the user has edited (with new changes) this action.
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


void impl::ClipboardActionEditor::NotifyPropertyChanged()
{
    /* TODO: As this control needs to implement INotifyPropertyChanged(text blocks will have their content changed), make this function auto raise the event.*/
}

