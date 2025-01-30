#pragma once
#include "SettingsPage.g.h"

#include "lib/Settings.hpp"
#include "lib/utils/Logger.hpp"
#include "lib/ui/VisualStateManager.hpp"

namespace winrt::ClipboardManager::implementation
{
    struct SettingsPage : SettingsPageT<SettingsPage>
    {
    public:
        SettingsPage();
        SettingsPage(const winrt::ClipboardManager::MainPage& mainPage);

    private:
        clip::ui::VisualState<SettingsPage> clearSettingsDefaultState{ L"ClearSettingsDefault", 0, true };
        clip::ui::VisualState<SettingsPage> clearSettingsShowIconState{ L"ClearSettingsShowIcon", 0, false };
        clip::ui::VisualState<SettingsPage> testWindowShortcutDefaultState{ L"TestWindowShortcutDefault", 1, true };
        clip::ui::VisualState<SettingsPage> testWindowShortcutState{ L"TestWindowShortcut", 1, false };
        clip::ui::VisualState<SettingsPage> testWindowShortcutOkState{ L"TestWindowShortcutOk", 1, false };
        clip::ui::VisualState<SettingsPage> testWindowShortcutNokState{ L"TestWindowShortcutNok", 1, false };
        clip::ui::VisualState<SettingsPage> logFilePathOkState{ L"LogFilePathOk", 2, false };
        clip::ui::VisualState<SettingsPage> logFilePathNokState{ L"LogFilePathNok", 2, false };
        clip::ui::VisualState<SettingsPage> logFilePathEmptyState{ L"LogFilePathEmpty", 2, true };
        clip::Settings settings{};
        bool loaded = false;
        clip::utils::Logger logger{ L"SettingsPage" };
        clip::ui::VisualStateManager<SettingsPage> visualStateManager{ *this };
        winrt::ClipboardManager::MainPage mainPage{ nullptr };

        void updateSetting(winrt::Windows::Foundation::IInspectable const& s, const std::wstring& key);
        void selectComboBoxItem(const winrt::Microsoft::UI::Xaml::Controls::ComboBox& comboBox, const uint32_t& value);
        uint32_t getSelectedComboBoxItemTag(const winrt::Microsoft::UI::Xaml::Controls::ComboBox& comboBox);

    public:
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
        void SaveWindowShortcutButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void DoubleAnimation_Completed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
        void OverlayResizableToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void OverlayShownInSwitcherToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ClipboardActionViewClickComboBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void LogFileTextBox_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::TextChangedEventArgs const& e);
        void DoubleAnimation_Completed_1(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
        void ClipboardEventingToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void LoggingToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::ClipboardManager::factory_implementation
{
    struct SettingsPage : SettingsPageT<SettingsPage, implementation::SettingsPage>
    {
    };
}
