#pragma once
#include "ClipboardActionEditor.g.h"

#include "src/ui/VisualStateManager.hpp"

namespace winrt::ClipboardManager::implementation
{
    struct ClipboardActionEditor : ClipboardActionEditorT<ClipboardActionEditor>
    {
    public:
        ClipboardActionEditor();
        
        winrt::hstring ActionLabel() const;
        void ActionLabel(const winrt::hstring& value);
        winrt::hstring ActionFormat() const;
        void ActionFormat(const winrt::hstring& value);
        winrt::hstring ActionRegex() const;
        void ActionRegex(const winrt::hstring& value);

        void RemoveButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::async EditButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ActionEnabledToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void UserControl_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        bool loaded;
        winrt::hstring _actionLabel;
        winrt::hstring _actionFormat;
        winrt::hstring _actionRegex;
        clipmgr::ui::VisualStateManager<ClipboardActionEditor> visualStateManager{ *this };
        const clipmgr::ui::VisualState<ClipboardActionEditor> EnabledState{ L"Enabled", 0, true };
        const clipmgr::ui::VisualState<ClipboardActionEditor> DisabledState{ L"Disabled", 0, false };

        void NotifyPropertyChanged();
    };
}

namespace winrt::ClipboardManager::factory_implementation
{
    struct ClipboardActionEditor : ClipboardActionEditorT<ClipboardActionEditor, implementation::ClipboardActionEditor>
    {
    };
}
