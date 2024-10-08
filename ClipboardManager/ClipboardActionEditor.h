#pragma once
#include "ClipboardActionEditor.g.h"

#include "src/ui/VisualStateManager.hpp"
#include "src/ui/ListenablePropertyValue.hpp"
#include "src/utils/Logger.hpp"
#include "src/utils/ResLoader.hpp"

#include <atomic>
#include <functional>

namespace winrt::ClipboardManager::implementation
{
    struct ClipboardActionEditor : ClipboardActionEditorT<ClipboardActionEditor>, clip::ui::PropertyChangedClass
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
        bool IgnoreCase() const;
        void IgnoreCase(const bool& value);
        bool UseSearch() const;
        void UseSearch(const bool& value);

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
        winrt::Windows::Foundation::IAsyncOperation<bool> Edit();

        void RemoveButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::async EditButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ActionEnabledToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void UserControl_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void TeachingTip_CloseButtonClick(winrt::Microsoft::UI::Xaml::Controls::TeachingTip const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void RootGrid_PointerEntered(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void RootGrid_PointerExited(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);

    private:
        clip::ui::VisualStateManager<ClipboardActionEditor> visualStateManager{ *this };
        const clip::ui::VisualState<ClipboardActionEditor> EnabledState{ L"Enabled", 0, true };
        const clip::ui::VisualState<ClipboardActionEditor> DisabledState{ L"Disabled", 0, false };
        // Label error states:
        const clip::ui::VisualState<ClipboardActionEditor> NoLabelErrorState{ L"NoLabelError", 1, true };
        const clip::ui::VisualState<ClipboardActionEditor> LabelErrorState{ L"LabelError", 1, false };
        // Format error states:
        const clip::ui::VisualState<ClipboardActionEditor> NoFormatErrorState{ L"NoFormatError", 2, true };
        const clip::ui::VisualState<ClipboardActionEditor> FormatErrorState{ L"FormatError", 2, false };
        const clip::ui::VisualState<ClipboardActionEditor> FormatWarningState{ L"FormatWarning", 2, false };
        // Regex error states:
        const clip::ui::VisualState<ClipboardActionEditor> NoRegexErrorState{ L"NoRegexError", 3, true };
        const clip::ui::VisualState<ClipboardActionEditor> RegexErrorState{ L"RegexError", 3, false };
        // Trigger options states:
        const clip::ui::VisualState<ClipboardActionEditor> IgnoreCaseDisabledState{ L"IgnoreCaseDisabled", 4, true };
        const clip::ui::VisualState<ClipboardActionEditor> IgnoreCaseEnabledState{ L"IgnoreCaseEnabled", 4, false };
        const clip::ui::VisualState<ClipboardActionEditor> UseSearchEnabledState{ L"UseSearchEnabled", 5, true };
        const clip::ui::VisualState<ClipboardActionEditor> UseSearchDisabledState{ L"UseSearchDisabled", 5, false };

        const clip::utils::Logger logger{ L"ClipboardActionEditor" };

        bool loaded = false;
        std::atomic_flag waitFlag{};
        clip::utils::ResLoader resLoader{};
        clip::ui::ListenablePropertyValue<bool> _ignoreCaseProperty{ std::bind(&ClipboardActionEditor::ignoreCasePropertyChanged, this, std::placeholders::_1) };
        clip::ui::ListenablePropertyValue<bool> _useSearchProperty{ std::bind(&ClipboardActionEditor::useSearchPropertyChanged, this, std::placeholders::_1) };
        clip::ui::ListenablePropertyValue<bool> _actionEnabledProperty{ std::bind(&ClipboardActionEditor::raisePropertyChanged, this, std::placeholders::_1) };
        clip::ui::ListenablePropertyValue<winrt::hstring> _triggerLabelProperty{ std::bind(&ClipboardActionEditor::raisePropertyChanged, this, std::placeholders::_1) };
        clip::ui::ListenablePropertyValue<winrt::hstring> _triggerFormatProperty{ std::bind(&ClipboardActionEditor::raisePropertyChanged, this, std::placeholders::_1) };
        clip::ui::ListenablePropertyValue<winrt::hstring> _triggerRegexProperty{ std::bind(&ClipboardActionEditor::raisePropertyChanged, this, std::placeholders::_1) };

        winrt::event<event_is_on_t> e_isOn{};
        winrt::event<event_changed_t> e_labelChanged{};
        winrt::event<event_changed_t> e_formatChanged{};
        winrt::event<event_re_changed_t> e_regexChanged{};
        winrt::event<event_removed_t> e_removed{};

        void useSearchPropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs args);
        void ignoreCasePropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs args);

        // Inherited via PropertyChangedClass
        winrt::Windows::Foundation::IInspectable asInspectable() override;
    };
}

namespace winrt::ClipboardManager::factory_implementation
{
    struct ClipboardActionEditor : ClipboardActionEditorT<ClipboardActionEditor, implementation::ClipboardActionEditor>
    {
    };
}
