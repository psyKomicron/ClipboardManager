#pragma once
#include "RegexTesterControl.g.h"

#include "lib/ui/VisualStateManager.hpp"
#include "lib/utils/Logger.hpp"

namespace winrt::ClipboardManager::implementation
{
    struct RegexTesterControl : RegexTesterControlT<RegexTesterControl>
    {
    public:
        RegexTesterControl() = default;

        winrt::async Open();

    private:
        clip::ui::VisualStateManager<RegexTesterControl> visualStateManager{ *this };
        clip::ui::VisualState<RegexTesterControl> noErrorState{ L"NoError", 0, true };
        clip::ui::VisualState<RegexTesterControl> errorState{ L"Error", 0, false };
        clip::utils::Logger logger{ L"RegexTesterControl" };

        void refreshMatches();

    public:
        void TestRegexContentDialog_CloseButtonClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RegexTextBox_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::TextChangedEventArgs const& e);
        void TestInputTextBox_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::TextChangedEventArgs const& e);
        void ToggleButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

        /*void RegexIgnoreCaseToggleButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RegexModeToggleButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);*/
        void FormatTextBox_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::TextChangedEventArgs const& e);
    };
}

namespace winrt::ClipboardManager::factory_implementation
{
    struct RegexTesterControl : RegexTesterControlT<RegexTesterControl, implementation::RegexTesterControl>
    {
    };
}
