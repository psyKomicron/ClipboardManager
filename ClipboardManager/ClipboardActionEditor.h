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
    private:
        using event_changed_t = winrt::Windows::Foundation::TypedEventHandler<winrt::ClipboardManager::ClipboardActionEditor, winrt::Windows::Foundation::IInspectable>;
        using event_label_changed_t = winrt::Windows::Foundation::TypedEventHandler<winrt::ClipboardManager::ClipboardActionEditor, hstring>;
        using event_is_on_t = winrt::Windows::Foundation::TypedEventHandler<winrt::ClipboardManager::ClipboardActionEditor, bool>;

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
        bool UseRegexMatchResults() const;
        void UseRegexMatchResults(const bool& value);

        winrt::event_token IsOn(const event_is_on_t& handler);
        void IsOn(const winrt::event_token& token);
        winrt::event_token Removed(const event_changed_t& handler);
        void Removed(const winrt::event_token& token);

        winrt::event_token LabelChanged(const event_label_changed_t& handler);
        void LabelChanged(const winrt::event_token& token);
        winrt::event_token Changed(const event_changed_t& handler);
        void Changed(const winrt::event_token& token);

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
        static clip::utils::ResLoader resLoader;
        // Const attributes:
        const clip::utils::Logger logger{ L"ClipboardActionEditor" };
        // Visual states:
        clip::ui::VisualState<ClipboardActionEditor> enabledState{ L"Enabled", 0, true };
        clip::ui::VisualState<ClipboardActionEditor> disabledState{ L"Disabled", 0, false };
        //    Label error states:
        clip::ui::VisualState<ClipboardActionEditor> noLabelErrorState{ L"NoLabelError", 1, true };
        clip::ui::VisualState<ClipboardActionEditor> labelErrorState{ L"LabelError", 1, false };
        //    Format error states:
        clip::ui::VisualState<ClipboardActionEditor> noFormatErrorState{ L"NoFormatError", 2, true };
        clip::ui::VisualState<ClipboardActionEditor> formatErrorState{ L"FormatError", 2, false };
        clip::ui::VisualState<ClipboardActionEditor> formatWarningState{ L"FormatWarning", 2, false };
        //    Regex error states:
        clip::ui::VisualState<ClipboardActionEditor> noRegexErrorState{ L"NoRegexError", 3, true };
        clip::ui::VisualState<ClipboardActionEditor> regexErrorState{ L"RegexError", 3, false };
        //    Trigger options states:
        clip::ui::VisualState<ClipboardActionEditor> ignoreCaseDisabledState{ L"IgnoreCaseDisabled", 4, true };
        clip::ui::VisualState<ClipboardActionEditor> ignoreCaseEnabledState{ L"IgnoreCaseEnabled", 4, false };
        clip::ui::VisualState<ClipboardActionEditor> useSearchEnabledState{ L"UseSearchEnabled", 5, true };
        clip::ui::VisualState<ClipboardActionEditor> useSearchDisabledState{ L"UseSearchDisabled", 5, false };
        clip::ui::VisualState<ClipboardActionEditor> useRegexMatchResultsDisabledState{ L"UseRegexMatchResultsDisabled", 6, true };
        clip::ui::VisualState<ClipboardActionEditor> useRegexMatchResultsEnabledState{ L"UseRegexMatchResultsEnabled", 6, false };
        // Non-const attributes:
        clip::ui::VisualStateManager<ClipboardActionEditor> visualStateManager{ *this };
        bool loaded = false;
        std::atomic_flag waitFlag{};
        // Properties:
        bool _ignoreCaseProperty{};
        bool _useSearchProperty{};
        bool _useRegexMatchResultsProperty{};
        clip::ui::ListenablePropertyValue<bool> _actionEnabledProperty{ std::bind(&ClipboardActionEditor::raisePropertyChanged, this, std::placeholders::_1) };
        clip::ui::ListenablePropertyValue<winrt::hstring> _triggerLabelProperty{ std::bind(&ClipboardActionEditor::raisePropertyChanged, this, std::placeholders::_1) };
        clip::ui::ListenablePropertyValue<winrt::hstring> _triggerFormatProperty{ std::bind(&ClipboardActionEditor::raisePropertyChanged, this, std::placeholders::_1) };
        clip::ui::ListenablePropertyValue<winrt::hstring> _triggerRegexProperty{ std::bind(&ClipboardActionEditor::raisePropertyChanged, this, std::placeholders::_1) };
        // Events:
        winrt::event<event_is_on_t> e_enabled{};
        winrt::event<event_changed_t> e_removed{};
        winrt::event<event_label_changed_t> e_labelChanged{};
        winrt::event<event_changed_t> e_changed{};

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
