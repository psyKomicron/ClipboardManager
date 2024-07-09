#pragma once
#include "ClipboardActionEditor.g.h"

#include "src/ui/VisualStateManager.hpp"
#include "src/utils/Logger.hpp"

#include <atomic>

namespace winrt::ClipboardManager::implementation
{
    struct ClipboardActionEditor : ClipboardActionEditorT<ClipboardActionEditor>
    {
        using event_changed_t = winrt::Windows::Foundation::TypedEventHandler<winrt::ClipboardManager::ClipboardActionEditor, winrt::hstring>;
        using event_is_on_t = winrt::Windows::Foundation::TypedEventHandler<winrt::ClipboardManager::ClipboardActionEditor, bool>;
        using event_re_changed_t = winrt::Windows::Foundation::TypedEventHandler<winrt::ClipboardManager::ClipboardActionEditor, winrt::Windows::Foundation::IInspectable>;
        using event_removed_t = winrt::Windows::Foundation::TypedEventHandler<winrt::ClipboardManager::ClipboardActionEditor, winrt::Windows::Foundation::IInspectable>;

    public:
        ClipboardActionEditor();
        
        winrt::hstring ActionLabel() const;
        void ActionLabel(const winrt::hstring& value);
        winrt::hstring ActionFormat() const;
        void ActionFormat(const winrt::hstring& value);
        winrt::hstring ActionRegex() const;
        void ActionRegex(const winrt::hstring& value);
        bool ActionEnabled() const;
        void ActionEnabled(const bool& value);

        event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& value)
        {
            return e_propertyChanged.add(value);
        };

        void PropertyChanged(winrt::event_token const& token)
        {
            e_propertyChanged.remove(token);
        };

        winrt::event_token IsOn(const event_is_on_t& handler);
        void IsOn(const winrt::event_token& token);
        winrt::event_token LabelChanged(const event_changed_t& handler);
        void LabelChanged(const winrt::event_token& token);
        winrt::event_token FormatChanged(const event_changed_t& handler);
        void FormatChanged(const winrt::event_token& token);
        winrt::event_token RegexChanged(const event_re_changed_t& handler);
        void RegexChanged(const winrt::event_token& token);
        winrt::event_token Removed(const event_removed_t& handler);
        void Removed(const winrt::event_token& token);

        winrt::async StartTour();

        void RemoveButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::async EditButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ActionEnabledToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void UserControl_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void TeachingTip_CloseButtonClick(winrt::Microsoft::UI::Xaml::Controls::TeachingTip const& sender, winrt::Windows::Foundation::IInspectable const& args);

    private:
        const clipmgr::ui::VisualState<ClipboardActionEditor> EnabledState{ L"Enabled", 0, true };
        const clipmgr::ui::VisualState<ClipboardActionEditor> DisabledState{ L"Disabled", 0, false };
        const clipmgr::utils::Logger logger{ L"ClipboardActionEditor" };
        clipmgr::ui::VisualStateManager<ClipboardActionEditor> visualStateManager{ *this };
        bool loaded = false;
        std::atomic_flag waitFlag{};
        winrt::hstring _actionLabel{};
        winrt::hstring _actionFormat{};
        winrt::hstring _actionRegex{};
        bool _actionEnabled{};
        bool _useSearch = false;
        bool _ignoreCase = false;
        winrt::event<event_is_on_t> e_isOn{};
        winrt::event<event_changed_t> e_labelChanged{};
        winrt::event<event_changed_t> e_formatChanged{};
        winrt::event<event_re_changed_t> e_regexChanged{};
        winrt::event<event_removed_t> e_removed{};
        winrt::event<winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> e_propertyChanged{};

        void NotifyPropertyChanged(std::source_location sourceLocation = std::source_location::current());
    };
}

namespace winrt::ClipboardManager::factory_implementation
{
    struct ClipboardActionEditor : ClipboardActionEditorT<ClipboardActionEditor, implementation::ClipboardActionEditor>
    {
    };
}
