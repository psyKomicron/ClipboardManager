#pragma once
#include "SettingsPage.g.h"

#include "src/Settings.hpp"
#include "src/utils/Logger.hpp"
#include "src/ui/VisualStateManager.hpp"

namespace winrt::ClipboardManager::implementation
{
    struct SettingsPage : SettingsPageT<SettingsPage>
    {
    public:
        SettingsPage() = default;
        SettingsPage(const winrt::ClipboardManager::MainPage& mainPage);

        void AutoStartToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void Page_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void Page_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void StartMinimizedToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void SaveMatchingResultsToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void EnableListeningToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void DurationToggleButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void NotificationScenariosComboBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void NotificationSoundComboBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void IgnoreCaseToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RegexModeComboBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        winrt::async CreateExampleTriggersFileButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::async OverwriteExampleTriggersFileButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void SaveBrowserStringButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void BrowserStringTextBox_LostFocus(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void BrowserStringTextBox_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::TextChangedEventArgs const& e);
        void UseCustomBrowser_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void HideAppWindowToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ClearSettingsButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void TriggersStorageExpander_Expanding(winrt::Microsoft::UI::Xaml::Controls::Expander const& sender, winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs const& args);
        winrt::async LocateButton_Clicked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void AddDuplicatedActionsToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ImportClipboardHistoryToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void AllowMinimizeToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void AllowMaximizeToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void EnableFileWatchingToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        const clip::ui::VisualState<SettingsPage> ClearSettingsDefaultState{ L"ClearSettingsDefault", 0, true };
        const clip::ui::VisualState<SettingsPage> ClearSettingsShowIconState{ L"ClearSettingsShowIcon", 0, false };
        clip::Settings settings{};
        bool loaded = false;
        clip::utils::Logger logger{ L"SettingsPage" };
        clip::ui::VisualStateManager<SettingsPage> visualStateManager{ *this };
        winrt::ClipboardManager::MainPage mainPage{ nullptr };

        void updateSetting(winrt::Windows::Foundation::IInspectable const& s, const std::wstring& key);
        void selectComboBoxItem(const winrt::Microsoft::UI::Xaml::Controls::ComboBox& comboBox, const uint32_t& value);
        uint32_t getSelectedComboBoxItemTag(const winrt::Microsoft::UI::Xaml::Controls::ComboBox& comboBox);
    };
}

namespace winrt::ClipboardManager::factory_implementation
{
    struct SettingsPage : SettingsPageT<SettingsPage, implementation::SettingsPage>
    {
    };
}
