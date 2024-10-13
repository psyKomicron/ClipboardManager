#include "pch.h"
#include "SettingsPage.h"
#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif

#include "src/utils/StartupTask.hpp"
#include "src/utils/helpers.hpp"
#include "src/notifs/NotificationTypes.hpp"
#include "src/ClipboardTrigger.hpp"
#include "Resource.h"

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Windows.Storage.Pickers.h>

#include <Shobjidl.h>

namespace winrt
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
    SettingsPage::SettingsPage(const winrt::ClipboardManager::MainPage& mainPage) :
        mainPage{ mainPage }
    {
    }

    void SettingsPage::Page_Loading(winrt::FrameworkElement const&, winrt::IInspectable const&)
    {
        ApplicationVersionHostControl().HostContent(box_value(APP_VERSION));

        // Application:
        clip::utils::StartupTask startupTask{};
        AutoStartToggleSwitch().IsOn(startupTask.isTaskRegistered());

        SaveMatchingResultsToggleSwitch().IsOn(settings.get<bool>(L"SaveMatchingResults").value_or(false));
        StartMinimizedToggleSwitch().IsOn(settings.get<bool>(L"StartWindowMinimized").value_or(false));
        NotificationsToggleSwitch().IsOn(settings.get<bool>(L"NotificationsEnabled").value_or(true));

        // Formatting & matching:
        IgnoreCaseToggleSwitch().IsOn(settings.get<bool>(L"RegexIgnoreCase").value_or(false));
        selectComboBoxItem(RegexModeComboBox(), settings.get<int32_t>(L"TriggerMatchMode").value_or(0));
        AddDuplicatedActionsToggleSwitch().IsOn(settings.get<bool>(L"AddDuplicatedActions").value_or(true));
        ImportClipboardHistoryToggleSwitch().IsOn(settings.get<bool>(L"ImportClipboardHistory").value_or(false));

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
    }

    void SettingsPage::Page_Loaded(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
    {
        loaded = true;
    }

    void SettingsPage::AutoStartToggleSwitch_Toggled(winrt::IInspectable const& s, winrt::RoutedEventArgs const&)
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

    void SettingsPage::StartMinimizedToggleSwitch_Toggled(winrt::IInspectable const& s, xaml::RoutedEventArgs const&)
    {
        check_loaded(loaded);
        updateSetting(s, L"StartWindowMinimized");
    }

    void SettingsPage::SaveMatchingResultsToggleSwitch_Toggled(winrt::IInspectable const& s, xaml::RoutedEventArgs const&)
    {
        check_loaded(loaded);
        updateSetting(s, L"SaveMatchingResults");
    }

    void SettingsPage::EnableListeningToggleSwitch_Toggled(winrt::IInspectable const& s, xaml::RoutedEventArgs const&)
    {
        NotificationsExpander().IsEnabled(NotificationsToggleSwitch().IsOn());

        check_loaded(loaded);
        updateSetting(s, L"NotificationsEnabled");
    }

    void SettingsPage::DurationToggleButton_Click(winrt::IInspectable const& sender, xaml::RoutedEventArgs const& e)
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

    void SettingsPage::NotificationScenariosComboBox_SelectionChanged(winrt::IInspectable const&, xaml::SelectionChangedEventArgs const& e)
    {
        check_loaded(loaded);
        settings.insert(L"NotificationScenarioType", getSelectedComboBoxItemTag(NotificationScenariosComboBox()));
    }

    void SettingsPage::NotificationSoundComboBox_SelectionChanged(winrt::IInspectable const&, xaml::SelectionChangedEventArgs const& e)
    {
        check_loaded(loaded);
        settings.insert(L"NotificationSoundType", getSelectedComboBoxItemTag(NotificationSoundComboBox()));
    }

    void SettingsPage::IgnoreCaseToggleSwitch_Toggled(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
    {
        check_loaded(loaded);
        settings.insert(L"RegexIgnoreCase", IgnoreCaseToggleSwitch().IsOn());
    }

    void SettingsPage::RegexModeComboBox_SelectionChanged(winrt::IInspectable const&, xaml::SelectionChangedEventArgs const&)
    {
        check_loaded(loaded);
        settings.insert(L"TriggerMatchMode", RegexModeComboBox().SelectedItem().as<winrt::FrameworkElement>().Tag().as<int32_t>());
    }

    winrt::async SettingsPage::CreateExampleTriggersFileButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
    {
        winrt::FileSavePicker picker{};
        picker.as<IInitializeWithWindow>()->Initialize(GetActiveWindow());
        picker.SuggestedStartLocation(winrt::PickerLocationId::ComputerFolder);
        picker.SuggestedFileName(L"user_file.xml");
        picker.FileTypeChoices().Insert(
            L"XML Files", single_threaded_vector<winrt::hstring>({ L".xml" })
        );

        auto&& storageFile = co_await picker.PickSaveFileAsync();
        if (storageFile)
        {
            std::filesystem::path userFilePath{ storageFile.Path().c_str() };
            clip::ClipboardTrigger::initializeSaveFile(userFilePath);
        }
    }

    winrt::async SettingsPage::OverwriteExampleTriggersFileButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
    {
        winrt::FileOpenPicker picker{};
        picker.as<IInitializeWithWindow>()->Initialize(GetActiveWindow());
        picker.FileTypeFilter().Append(L".xml");

        auto&& storageFile = co_await picker.PickSingleFileAsync();
        if (storageFile)
        {
            std::filesystem::path userFilePath{ storageFile.Path().c_str() };
            settings.insert(L"UserFilePath", userFilePath);
            clip::ClipboardTrigger::initializeSaveFile(userFilePath);
        }
    }

    void SettingsPage::SaveBrowserStringButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
    {
        BrowserStringTextBox_TextChanged(nullptr, nullptr);
    }

    void SettingsPage::BrowserStringTextBox_TextChanged(winrt::IInspectable const&, xaml::TextChangedEventArgs const&)
    {
        check_loaded(loaded);
        // TODO: Check if writing that fast to the registry can hurt performance.
        settings.insert(L"CustomProcessString", BrowserStringTextBox().Text());
    }

    void SettingsPage::UseCustomBrowser_Toggled(winrt::IInspectable const& sender, winrt::RoutedEventArgs const&)
    {
        check_loaded(loaded);
        updateSetting(sender, L"UseCustomProcess");
    }

    void SettingsPage::HideAppWindowToggleSwitch_Toggled(winrt::IInspectable const& sender, winrt::RoutedEventArgs const&)
    {
        check_loaded(loaded);
        updateSetting(sender, L"HideAppWindow");
    }

    void SettingsPage::ClearSettingsButton_Click(winrt::IInspectable const&, xaml::RoutedEventArgs const& e)
    {
        settings.clear();
        visualStateManager.goToState(ClearSettingsShowIconState);
    }

    void SettingsPage::TriggersStorageExpander_Expanding(xaml::Controls::Expander const&, xaml::Controls::ExpanderExpandingEventArgs const&)
    {
        UserFilePathTextBlock().Text(settings.get<hstring>(L"UserFilePath").value_or(L""));
    }

    winrt::async SettingsPage::LocateButton_Clicked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        FileOpenPicker picker{};
        picker.as<IInitializeWithWindow>()->Initialize(GetActiveWindow());
        picker.FileTypeFilter().Append(L".xml");

        auto&& storageFile = co_await picker.PickSingleFileAsync();
        if (storageFile)
        {
            std::filesystem::path userFilePath{ storageFile.Path().c_str() };
            settings.insert(L"UserFilePath", userFilePath);
        }
    }

    void SettingsPage::AddDuplicatedActionsToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const& sender, xaml::RoutedEventArgs const& e)
    {
        check_loaded(loaded);
        updateSetting(sender, L"AddDuplicatedActions");
    }

    void SettingsPage::ImportClipboardHistoryToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const& sender, xaml::RoutedEventArgs const& e)
    {
        check_loaded(loaded);
        updateSetting(sender, L"ImportClipboardHistory");
    }

    void SettingsPage::AllowMinimizeToggleSwitch_Toggled(Windows::Foundation::IInspectable const& sender, xaml::RoutedEventArgs const&)
    {
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
        }
    }

    void SettingsPage::AllowMaximizeToggleSwitch_Toggled(Windows::Foundation::IInspectable const& sender, xaml::RoutedEventArgs const&)
    {
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


    void SettingsPage::updateSetting(winrt::Windows::Foundation::IInspectable const& s, const std::wstring& key)
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
}
