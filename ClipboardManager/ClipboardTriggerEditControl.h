#pragma once
#include "ClipboardTriggerEditControl.g.h"

#include "src/ui/ListenablePropertyValue.hpp"
#include "src/ui/VisualStateManager.hpp"
#include "src/utils/ResLoader.hpp"
#include "src/utils/Logger.hpp"

namespace winrt::ClipboardManager::implementation
{
    struct ClipboardTriggerEditControl : ClipboardTriggerEditControlT<ClipboardTriggerEditControl>, clip::ui::PropertyChangedClass
    {
    public:
        ClipboardTriggerEditControl();

        winrt::hstring TriggerLabel() const;
        void TriggerLabel(const winrt::hstring& value);
        winrt::hstring TriggerFormat() const;
        void TriggerFormat(const winrt::hstring& value);
        winrt::hstring TriggerRegex() const;
        void TriggerRegex(const winrt::hstring& value);
        bool IgnoreCase() const;
        void IgnoreCase(const bool& value);
        bool UseSearch() const;
        void UseSearch(const bool& value);
        bool UseRegexMatchResults() const;
        void UseRegexMatchResults(const bool& value);

        winrt::Windows::Foundation::IAsyncOperation<bool> Edit();

        void TextBox_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::TextChangedEventArgs const& e);
        void EditDialog_Opened(winrt::Microsoft::UI::Xaml::Controls::ContentDialog const& sender, winrt::Microsoft::UI::Xaml::Controls::ContentDialogOpenedEventArgs const& args);

    private:
        static clip::utils::ResLoader resLoader;
        clip::utils::Logger logger{ L"ClipboardTriggerEditControl" };

        clip::ui::VisualStateManager<ClipboardTriggerEditControl> visualStateManager{ *this };
        clip::ui::VisualState<ClipboardTriggerEditControl> regexErrorState{ L"RegexError", 0, false };
        clip::ui::VisualState<ClipboardTriggerEditControl> regexNoErrorState{ L"RegexNoError", 0, false };
        clip::ui::VisualState<ClipboardTriggerEditControl> labelErrorState{ L"LabelError", 1, false };
        clip::ui::VisualState<ClipboardTriggerEditControl> labelNoErrorState{ L"LabelNoError", 1, false };
        clip::ui::VisualState<ClipboardTriggerEditControl> formatErrorState{ L"FormatError", 2, false };
        clip::ui::VisualState<ClipboardTriggerEditControl> formatNoErrorState{ L"FormatNoError", 2, false };
        clip::ui::VisualState<ClipboardTriggerEditControl> formatWarningState{ L"FormatWarning", 3, false };
        clip::ui::VisualState<ClipboardTriggerEditControl> formatNoWarningState{ L"FormatNoWarning", 3, false };
        clip::ui::VisualState<ClipboardTriggerEditControl> formatProtocolWarningState{ L"FormatProtocolWarning", 3, false };
        clip::ui::VisualState<ClipboardTriggerEditControl> formatNoProtocolWarningState{ L"FormatNoProtocolWarning", 3, false };

        clip::ui::ListenablePropertyValue<hstring> _triggerLabel{ BIND(&ClipboardTriggerEditControl::raisePropertyChanged) };
        clip::ui::ListenablePropertyValue<hstring> _triggerFormat{ BIND(&ClipboardTriggerEditControl::raisePropertyChanged) };
        clip::ui::ListenablePropertyValue<hstring> _triggerRegex{ BIND(&ClipboardTriggerEditControl::raisePropertyChanged) };
        clip::ui::ListenablePropertyValue<bool> _ignoreCase{ BIND(&ClipboardTriggerEditControl::raisePropertyChanged) };
        clip::ui::ListenablePropertyValue<bool> _useSearch{ BIND(&ClipboardTriggerEditControl::raisePropertyChanged) };
        clip::ui::ListenablePropertyValue<bool> _useRegexMatchResults{ BIND(&ClipboardTriggerEditControl::raisePropertyChanged), true };

        // Inherited via PropertyChangedClass
        winrt::Windows::Foundation::IInspectable asInspectable() override;

        void CheckInput();
        bool CheckFormat();
        bool CheckRegex();
    };
}

namespace winrt::ClipboardManager::factory_implementation
{
    struct ClipboardTriggerEditControl : ClipboardTriggerEditControlT<ClipboardTriggerEditControl, implementation::ClipboardTriggerEditControl>
    {
    };
}
