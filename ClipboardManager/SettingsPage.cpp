#include "pch.h"
#include "SettingsPage.h"
#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif

#include "lib/ClipboardTrigger.hpp"
#include "lib/HotKey.hpp"
#include "lib/utils/StartupTask.hpp"
#include "lib/utils/helpers.hpp"
#include "lib/utils/AppVersion.hpp"
#include "lib/notifs/NotificationTypes.hpp"

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Windows.Storage.Pickers.h>

#include <Shobjidl.h>

#include <chrono>

namespace win
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Storage::Pickers;
}

namespace xaml
{
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Controls;
    using namespace winrt::Microsoft::UI::Xaml::Controls::Primitives;
}

namespace winrt::ClipboardManager::implementation
{
    SettingsPage::SettingsPage()
    {
        visualStateManager.initializeStates(
            {
                testWindowShortcutDefaultState,
                testWindowShortcutState,
                testWindowShortcutOkState,
                testWindowShortcutNokState
            });
    }

    SettingsPage::SettingsPage(const winrt::ClipboardManager::MainPage& mainPage) :
        SettingsPage()
    {
        this->mainPage = mainPage;
    }


    void SettingsPage::updateSetting(win::IInspectable const& s, const std::wstring& key)
    {
        auto sender = s.as<xaml::ToggleSwitch>();
        auto isOn = sender.IsOn();
        settings.insert(key, isOn);
    }

    void SettingsPage::selectComboBoxItem(const xaml::Controls::ComboBox& comboBox, const uint32_t& value)
    {
        for (auto&& inspectable : comboBox.Items())
        {
            auto item = inspectable.try_as<xaml::ComboBoxItem>();
            if (item && item.Tag().as<hstring>() == std::to_wstring(value))
            {
                comboBox.SelectedItem(inspectable);
                break;
            }
        }
    }

    uint32_t SettingsPage::getSelectedComboBoxItemTag(const xaml::Controls::ComboBox& comboBox)
    {
        return std::stoi(comboBox.SelectedItem().as<xaml::ComboBoxItem>().Tag().as<hstring>().data());
    }


    void SettingsPage::Page_Loading(xaml::FrameworkElement const&, win::IInspectable const&)
    {
        clip::utils::AppVersion appVersion{};
        ApplicationVersionHostControl().HostContent(box_value(appVersion.versionString() + L"  " + L'"' + appVersion.name() + L'"'));

        // Application:
        clip::utils::StartupTask startupTask{};
        AutoStartToggleSwitch().IsOn(startupTask.isTaskRegistered());

        SaveMatchingResultsToggleSwitch().IsOn(settings.get<bool>(L"SaveMatchingResults").value_or(false));
        StartMinimizedToggleSwitch().IsOn(settings.get<bool>(L"StartWindowMinimized").value_or(false));
        HideAppWindowToggleSwitch().IsOn(settings.get<bool>(L"HideAppWindow").value_or(false));
        NotificationsToggleSwitch().IsOn(settings.get<bool>(L"NotificationsEnabled").value_or(true));

        // Formatting & matching:
        IgnoreCaseToggleSwitch().IsOn(settings.get<bool>(L"RegexIgnoreCase").value_or(false));
        selectComboBoxItem(RegexModeComboBox(), settings.get<int32_t>(L"TriggerMatchMode").value_or(0));
        AddDuplicatedActionsToggleSwitch().IsOn(settings.get<bool>(L"AddDuplicatedActions").value_or(true));
        ImportClipboardHistoryToggleSwitch().IsOn(settings.get<bool>(L"ImportClipboardHistory").value_or(false));
        selectComboBoxItem(ClipboardActionViewClickComboBox(), settings.get<int32_t>(L"ClipboardActionClick").value_or(0));

        // Notifications:
        auto durationType = settings.get<clip::notifs::NotificationDurationType>(L"NotificationDurationType").value_or(clip::notifs::NotificationDurationType::Default);
        DurationDefaultToggleButton().IsChecked(durationType == clip::notifs::NotificationDurationType::Default);
        DurationShortToggleButton().IsChecked(durationType == clip::notifs::NotificationDurationType::Short);
        DurationLongToggleButton().IsChecked(durationType == clip::notifs::NotificationDurationType::Long);

        auto scenarioType = settings.get<clip::notifs::NotificationScenarioType>(L"NotificationScenarioType").value_or(clip::notifs::NotificationScenarioType::Default);
        auto soundType = settings.get<clip::notifs::NotificationSoundType>(L"NotificationSoundType").value_or(clip::notifs::NotificationSoundType::Default);
        selectComboBoxItem(NotificationScenariosComboBox(), (int32_t)scenarioType);
        selectComboBoxItem(NotificationSoundComboBox(), (int32_t)soundType);

        // Browser:
        BrowserStringTextBox().Text(settings.get<std::wstring>(L"CustomProcessString").value_or(L""));
        UseCustomBrowser().IsOn(settings.get<bool>(L"UseCustomProcess").value_or(false));

        // Window style:
        AllowMaximizeToggleSwitch().IsOn(settings.get<bool>(L"AllowWindowMaximize").value_or(false));
        AllowMinimizeToggleSwitch().IsOn(settings.get<bool>(L"AllowWindowMinimize").value_or(true));
        OverlayResizableToggleSwitch().IsOn(settings.get<bool>(L"OverlayIsResizable").value_or(true));
        OverlayShownInSwitcherToggleSwitch().IsOn(settings.get<bool>(L"OverlayShownInSwitchers").value_or(true));

        auto userFilePath = settings.get<hstring>(L"UserFilePath");
        if (userFilePath.has_value())
        {
            UserFilePathTextBlock().Text(userFilePath.value());
            UserFilePathTextBlock().FontStyle(Windows::UI::Text::FontStyle::Normal);
        }

        auto logFilePath = settings.get<std::filesystem::path>(L"LogFilePath");
        if (logFilePath && std::filesystem::exists(logFilePath.value()))
        {
            LogFileTextBox().Text(logFilePath.value().wstring());
        }

        loaded = true;
    }

    void SettingsPage::Page_Loaded(win::IInspectable const&, xaml::RoutedEventArgs const&)
    {
        //loaded = true;
    }

    void SettingsPage::AutoStartToggleSwitch_Toggled(win::IInspectable const& s, xaml::RoutedEventArgs const&)
    {
        check_loaded(loaded);
        auto sender = s.as<xaml::ToggleSwitch>();

        clip::utils::StartupTask startupTask{};
        if (!startupTask.isTaskRegistered() && sender.IsOn())
        {
            startupTask.set();
        }
        else if (startupTask.isTaskRegistered() && !sender.IsOn())
        {
            startupTask.remove();
        }
    }

    void SettingsPage::StartMinimizedToggleSwitch_Toggled(win::IInspectable const& s, xaml::RoutedEventArgs const&)
    {
        check_loaded(loaded);
        updateSetting(s, L"StartWindowMinimized");
    }

    void SettingsPage::SaveMatchingResultsToggleSwitch_Toggled(win::IInspectable const& s, xaml::RoutedEventArgs const&)
    {
        check_loaded(loaded);
        updateSetting(s, L"SaveMatchingResults");
    }

    void SettingsPage::EnableListeningToggleSwitch_Toggled(win::IInspectable const& s, xaml::RoutedEventArgs const&)
    {
        check_loaded(loaded);
        NotificationsExpander().IsEnabled(NotificationsToggleSwitch().IsOn());

        check_loaded(loaded);
        updateSetting(s, L"NotificationsEnabled");
    }

    void SettingsPage::DurationToggleButton_Click(win::IInspectable const& sender, xaml::RoutedEventArgs const& e)
    {
        auto senderToggleButton = sender.as<xaml::ToggleButton>();
        if (senderToggleButton.IsChecked())
        {
            for (auto&& child : DurationButtonsStackPanel().Children())
            {
                auto toggleButton = child.as<xaml::ToggleButton>();
                if (toggleButton.Tag().as<hstring>() != senderToggleButton.Tag().as<hstring>())
                {
                    toggleButton.IsChecked(false);
                }
            }
        }

        auto tag = std::stoi(std::wstring(senderToggleButton.Tag().as<hstring>()));
        settings.insert(L"NotificationDurationType", static_cast<clip::notifs::NotificationDurationType>(tag));
    }

    void SettingsPage::NotificationScenariosComboBox_SelectionChanged(win::IInspectable const&, xaml::SelectionChangedEventArgs const& e)
    {
        check_loaded(loaded);
        settings.insert(L"NotificationScenarioType", getSelectedComboBoxItemTag(NotificationScenariosComboBox()));
    }

    void SettingsPage::NotificationSoundComboBox_SelectionChanged(win::IInspectable const&, xaml::SelectionChangedEventArgs const& e)
    {
        check_loaded(loaded);
        settings.insert(L"NotificationSoundType", getSelectedComboBoxItemTag(NotificationSoundComboBox()));
    }

    void SettingsPage::IgnoreCaseToggleSwitch_Toggled(win::IInspectable const&, winrt::RoutedEventArgs const&)
    {
        check_loaded(loaded);
        settings.insert(L"RegexIgnoreCase", IgnoreCaseToggleSwitch().IsOn());
    }

    void SettingsPage::RegexModeComboBox_SelectionChanged(win::IInspectable const&, xaml::SelectionChangedEventArgs const&)
    {
        check_loaded(loaded);
        settings.insert(L"TriggerMatchMode", RegexModeComboBox().SelectedItem().as<winrt::FrameworkElement>().Tag().as<int32_t>());
    }

    winrt::async SettingsPage::CreateExampleTriggersFileButton_Click(win::IInspectable const&, winrt::RoutedEventArgs const&)
    {
        win::FileSavePicker picker{};
        picker.as<IInitializeWithWindow>()->Initialize(GetActiveWindow());
        picker.SuggestedStartLocation(win::PickerLocationId::ComputerFolder);
        picker.SuggestedFileName(L"user_file.xml");
        picker.FileTypeChoices().Insert(
            L"XML Files", single_threaded_vector<winrt::hstring>({ L".xml" })
        );

        auto&& storageFile = co_await picker.PickSaveFileAsync();
        if (storageFile)
        {
            std::filesystem::path userFilePath{ storageFile.Path().c_str() };
        }
    }

    winrt::async SettingsPage::OverwriteExampleTriggersFileButton_Click(win::IInspectable const&, winrt::RoutedEventArgs const&)
    {
        win::FileOpenPicker picker{};
        picker.as<IInitializeWithWindow>()->Initialize(GetActiveWindow());
        picker.FileTypeFilter().Append(L".xml");

        auto&& storageFile = co_await picker.PickSingleFileAsync();
        if (storageFile)
        {
            std::filesystem::path userFilePath{ storageFile.Path().c_str() };
            settings.insert(L"UserFilePath", userFilePath);
        }
    }

    void SettingsPage::SaveBrowserStringButton_Click(win::IInspectable const&, winrt::RoutedEventArgs const&)
    {
        BrowserStringTextBox_TextChanged(nullptr, nullptr);
    }

    void SettingsPage::BrowserStringTextBox_TextChanged(win::IInspectable const&, xaml::TextChangedEventArgs const&)
    {
        check_loaded(loaded);
        settings.insert(L"CustomProcessString", BrowserStringTextBox().Text());
    }

    void SettingsPage::UseCustomBrowser_Toggled(win::IInspectable const& sender, winrt::RoutedEventArgs const&)
    {
        check_loaded(loaded);
        updateSetting(sender, L"UseCustomProcess");
    }

    void SettingsPage::HideAppWindowToggleSwitch_Toggled(win::IInspectable const& sender, winrt::RoutedEventArgs const&)
    {
        check_loaded(loaded);
        updateSetting(sender, L"HideAppWindow");
    }

    void SettingsPage::ClearSettingsButton_Click(win::IInspectable const&, xaml::RoutedEventArgs const& e)
    {
        settings.clear();
        visualStateManager.goToState(clearSettingsShowIconState);
    }

    void SettingsPage::TriggersStorageExpander_Expanding(xaml::Controls::Expander const&, xaml::Controls::ExpanderExpandingEventArgs const&)
    {
    }

    winrt::async SettingsPage::LocateButton_Clicked(win::IInspectable const& sender, xaml::RoutedEventArgs const& e)
    {
        win::FileOpenPicker picker{};
        picker.as<IInitializeWithWindow>()->Initialize(GetActiveWindow());
        picker.FileTypeFilter().Append(L".xml");

        auto&& storageFile = co_await picker.PickSingleFileAsync();
        if (storageFile && mainPage)
        {
            mainPage.UpdateUserFile(storageFile.Path());
        }
        else if (!mainPage)
        {
            logger.info(L"MainPage is null, impossible to notify that the user file path has changed.");
        }
    }

    void SettingsPage::AddDuplicatedActionsToggleSwitch_Toggled(win::IInspectable const& sender, xaml::RoutedEventArgs const& e)
    {
        check_loaded(loaded);
        updateSetting(sender, L"AddDuplicatedActions");
    }

    void SettingsPage::ImportClipboardHistoryToggleSwitch_Toggled(win::IInspectable const& sender, xaml::RoutedEventArgs const& e)
    {
        check_loaded(loaded);
        updateSetting(sender, L"ImportClipboardHistory");
    }

    void SettingsPage::AllowMinimizeToggleSwitch_Toggled(win::IInspectable const& sender, xaml::RoutedEventArgs const&)
    {
        check_loaded(loaded);
        updateSetting(sender, L"AllowWindowMinimize");

        auto&& appWindow = clip::utils::getCurrentAppWindow();
        auto&& presenter = appWindow.Presenter().try_as<Microsoft::UI::Windowing::OverlappedPresenter>();
        if (presenter)
        {
            presenter.IsMinimizable(AllowMinimizeToggleSwitch().IsOn());

            if (mainPage)
            {
                mainPage.UpdateTitleBar();
            }
            else
            {
                logger.info(L"MainPage is null, impossible to update title bar.");
            }
        }
    }

    void SettingsPage::AllowMaximizeToggleSwitch_Toggled(win::IInspectable const& sender, xaml::RoutedEventArgs const&)
    {
        check_loaded(loaded);
        updateSetting(sender, L"AllowWindowMaximize");

        auto&& appWindow = clip::utils::getCurrentAppWindow();
        auto&& presenter = appWindow.Presenter().try_as<Microsoft::UI::Windowing::OverlappedPresenter>();
        if (presenter)
        {
            presenter.IsMaximizable(AllowMaximizeToggleSwitch().IsOn());
            if (mainPage)
            {
                mainPage.UpdateTitleBar();
            }
        }
    }

    void SettingsPage::EnableFileWatchingToggleSwitch_Toggled(win::IInspectable const& sender, xaml::RoutedEventArgs const&)
    {
        check_loaded(loaded);
        updateSetting(sender, L"EnableTriggerFileWatching");
    }

    void SettingsPage::SaveWindowShortcutButton_Click(win::IInspectable const&, xaml::RoutedEventArgs const&)
    {
        visualStateManager.goToState(testWindowShortcutState);

        bool success = false;

        try
        {
            auto selectedModifier = ModifierComboBox().SelectedIndex();
            auto selectedKey = KeyTextBox().Text();
            if (selectedModifier >= 0 && !selectedKey.empty())
            {
                uint32_t modifiers = 0;
                switch (selectedModifier)
                {
                    case 0:
                        modifiers = MOD_ALT;
                        break;
                    case 1:
                        modifiers = MOD_CONTROL;
                        break;
                    case 2:
                        modifiers = MOD_SHIFT;
                        break;
                }

                wchar_t key = selectedKey[0];

                clip::HotKey hotKey{ modifiers, key };
                hotKey.startListening([](){});
                success = true;
            }
        }
        catch (std::invalid_argument& invalidArg)
        {
            logger.error(L"HotKey - Invalid argument.");
        }

        visualStateManager.goToState(success ? testWindowShortcutOkState : testWindowShortcutNokState);
    }

    void SettingsPage::DoubleAnimation_Completed(win::IInspectable const&, win::IInspectable const&)
    {
        visualStateManager.goToState(testWindowShortcutDefaultState);
    }

    void SettingsPage::OverlayResizableToggleSwitch_Toggled(win::IInspectable const& sender, xaml::RoutedEventArgs const&)
    {
        check_loaded(loaded);
        updateSetting(sender, L"OverlayIsResizable");
    }

    void SettingsPage::OverlayShownInSwitcherToggleSwitch_Toggled(win::IInspectable const& sender, xaml::RoutedEventArgs const&)
    {
        check_loaded(loaded);
        updateSetting(sender, L"OverlayShownInSwitchers");
    }

    void SettingsPage::ClipboardActionViewClickComboBox_SelectionChanged(win::IInspectable const&, xaml::SelectionChangedEventArgs const&)
    {
        check_loaded(loaded);
        settings.insert(L"ClipboardActionClick", ClipboardActionViewClickComboBox().SelectedItem().as<winrt::FrameworkElement>().Tag().as<hstring>() == L"1");
    }

    void SettingsPage::LogFileTextBox_TextChanged(win::IInspectable const&, xaml::Controls::TextChangedEventArgs const&)
    {
        std::filesystem::path logFilePath{ std::wstring(LogFileTextBox().Text()) };
        if (std::filesystem::exists(logFilePath) && std::filesystem::is_directory(logFilePath))
        {
            visualStateManager.goToState(logFilePathOkState);
            TimeoutSaveStoryboard().Begin();

            return;
        }
        else if (!logFilePath.empty())
        {
            visualStateManager.goToState(logFilePathNokState);
        }
        else
        {
            visualStateManager.goToState(logFilePathEmptyState);
        }

        TimeoutSaveStoryboard().Stop();
        TimeoutSaveStoryboard().Seek(win::TimeSpan());
    }

    void SettingsPage::DoubleAnimation_Completed_1(win::IInspectable const&, win::IInspectable const&)
    {
        logger.debug(L"Saving user file path (timeout).");
        settings.insert<std::wstring_view>(L"LogFilePath", LogFileTextBox().Text());
    }
}
