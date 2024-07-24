#include "pch.h"
#include "SettingsPage.h"
#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif

#include "src/utils/StartupTask.hpp"
#include "src/utils/helpers.hpp"
#include "src/notifs/NotificationTypes.hpp"
#include "src/ClipboardTrigger.hpp"

//#include <boost/property_tree/ptree.hpp>
//#include <boost/property_tree/xml_parser.hpp>

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.Pickers.h>

#include <Shobjidl.h>

namespace implementation = winrt::ClipboardManager::implementation;
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

void implementation::SettingsPage::Page_Loading(winrt::FrameworkElement const&, winrt::IInspectable const&)
{
    clipmgr::utils::StartupTask startupTask{};
    AutoStartToggleSwitch().IsOn(startupTask.isTaskRegistered());

    SaveMatchingResultsToggleSwitch().IsOn(settings.get<bool>(L"SaveMatchingResults").value_or(false));
    StartMinimizedToggleSwitch().IsOn(settings.get<bool>(L"StartWindowMinimized").value_or(false));
    NotificationsToggleSwitch().IsOn(settings.get<bool>(L"NotificationsEnabled").value_or(false));
    
    auto durationType = settings.get<clipmgr::notifs::NotificationDurationType>(L"NotificationDurationType").value_or(clipmgr::notifs::NotificationDurationType::Default);
    DurationDefaultToggleButton().IsChecked(durationType == clipmgr::notifs::NotificationDurationType::Default);
    DurationShortToggleButton().IsChecked(durationType == clipmgr::notifs::NotificationDurationType::Short);
    DurationLongToggleButton().IsChecked(durationType == clipmgr::notifs::NotificationDurationType::Long);

    auto scenarioType = settings.get<clipmgr::notifs::NotificationScenarioType>(L"NotificationScenarioType").value_or(clipmgr::notifs::NotificationScenarioType::Default);
    auto soundType = settings.get<clipmgr::notifs::NotificationSoundType>(L"NotificationSoundType").value_or(clipmgr::notifs::NotificationSoundType::Default);
    selectComboBoxItem(NotificationScenariosComboBox(), (int32_t)scenarioType);
    selectComboBoxItem(NotificationSoundComboBox(), (int32_t)soundType);

    BrowserStringTextBox().Text(settings.get<std::wstring>(L"CustomProcessString").value_or(L""));
    UseCustomBrowser().IsOn(settings.get<bool>(L"UseCustomProcess").value_or(false));

    AllowMaximizeToggleSwitch().IsOn(settings.get<bool>(L"AllowWindowMaximize").value_or(false));
    AllowMinimizeToggleSwitch().IsOn(settings.get<bool>(L"AllowWindowMinimize").value_or(true));
}

void implementation::SettingsPage::Page_Loaded(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    loaded = true;
}

void implementation::SettingsPage::AutoStartToggleSwitch_Toggled(winrt::IInspectable const& s, winrt::RoutedEventArgs const&)
{
    check_loaded(loaded);
    auto sender = s.as<xaml::ToggleSwitch>();

    clipmgr::utils::StartupTask startupTask{};
    if (!startupTask.isTaskRegistered() && sender.IsOn())
    {
        startupTask.set();
    }
    else if (startupTask.isTaskRegistered() && !sender.IsOn())
    {
        startupTask.remove();
    }
}

void implementation::SettingsPage::StartMinimizedToggleSwitch_Toggled(winrt::IInspectable const& s, xaml::RoutedEventArgs const&)
{
    check_loaded(loaded);
    updateSetting(s, L"StartWindowMinimized");
}

void implementation::SettingsPage::SaveMatchingResultsToggleSwitch_Toggled(winrt::IInspectable const& s, xaml::RoutedEventArgs const&)
{
    check_loaded(loaded);
    updateSetting(s, L"SaveMatchingResults");
}

void implementation::SettingsPage::EnableListeningToggleSwitch_Toggled(winrt::IInspectable const& s, xaml::RoutedEventArgs const&)
{
    NotificationsExpander().IsEnabled(NotificationsToggleSwitch().IsOn());
    
    check_loaded(loaded);
    updateSetting(s, L"NotificationsEnabled");
}

void implementation::SettingsPage::DurationToggleButton_Click(winrt::IInspectable const& sender, xaml::RoutedEventArgs const& e)
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
    settings.insert(L"NotificationDurationType", static_cast<clipmgr::notifs::NotificationDurationType>(tag));
}

void implementation::SettingsPage::NotificationScenariosComboBox_SelectionChanged(winrt::IInspectable const&, xaml::SelectionChangedEventArgs const& e)
{
    check_loaded(loaded);
    settings.insert(L"NotificationScenarioType", getSelectedComboBoxItemTag(NotificationScenariosComboBox()));
}

void implementation::SettingsPage::NotificationSoundComboBox_SelectionChanged(winrt::IInspectable const&, xaml::SelectionChangedEventArgs const& e)
{
    check_loaded(loaded);
    settings.insert(L"NotificationSoundType", getSelectedComboBoxItemTag(NotificationSoundComboBox()));
}

void implementation::SettingsPage::IgnoreCaseToggleSwitch_Toggled(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    check_loaded(loaded);
    settings.insert(L"RegexIgnoreCase", IgnoreCaseToggleSwitch().IsOn());
}

void implementation::SettingsPage::RegexModeComboBox_SelectionChanged(winrt::IInspectable const&, xaml::SelectionChangedEventArgs const&)
{
    check_loaded(loaded);
    settings.insert(L"TriggerMatchMode", RegexModeComboBox().SelectedItem().as<winrt::FrameworkElement>().Tag().as<int32_t>());
}

winrt::async implementation::SettingsPage::CreateExampleTriggersFileButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
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
        clipmgr::ClipboardTrigger::initializeSaveFile(userFilePath);
    }
}

winrt::async implementation::SettingsPage::OverwriteExampleTriggersFileButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    winrt::FileOpenPicker picker{};
    picker.as<IInitializeWithWindow>()->Initialize(GetActiveWindow());
    picker.FileTypeFilter().Append(L".xml");

    auto&& storageFile = co_await picker.PickSingleFileAsync();
    if (storageFile)
    {
        std::filesystem::path userFilePath{ storageFile.Path().c_str() };
        settings.insert(L"UserFilePath", userFilePath);
        clipmgr::ClipboardTrigger::initializeSaveFile(userFilePath);
    }
}

void implementation::SettingsPage::SaveBrowserStringButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    BrowserStringTextBox_TextChanged(nullptr, nullptr);
}

void implementation::SettingsPage::BrowserStringTextBox_TextChanged(winrt::IInspectable const&, xaml::TextChangedEventArgs const&)
{
    check_loaded(loaded);
    // TODO: Check if writing that fast to the registry can hurt performance.
    settings.insert(L"CustomProcessString", BrowserStringTextBox().Text());
}

void implementation::SettingsPage::UseCustomBrowser_Toggled(winrt::IInspectable const& sender, winrt::RoutedEventArgs const&)
{
    check_loaded(loaded);
    updateSetting(sender, L"UseCustomProcess");
}

void implementation::SettingsPage::HideAppWindowToggleSwitch_Toggled(winrt::IInspectable const& sender, winrt::RoutedEventArgs const&)
{
    check_loaded(loaded);
    updateSetting(sender, L"HideAppWindow");
}

void implementation::SettingsPage::ClearSettingsButton_Click(winrt::IInspectable const&, xaml::RoutedEventArgs const& e)
{
    settings.clear();
    visualStateManager.goToState(ClearSettingsShowIconState);
}

void implementation::SettingsPage::TriggersStorageExpander_Expanding(xaml::Controls::Expander const&, xaml::Controls::ExpanderExpandingEventArgs const&)
{
    UserFilePathTextBlock().Text(settings.get<hstring>(L"UserFilePath").value_or(L""));
}


void implementation::SettingsPage::updateSetting(winrt::Windows::Foundation::IInspectable const& s, const std::wstring& key)
{
    auto sender = s.as<xaml::ToggleSwitch>();
    auto isOn = sender.IsOn();
    settings.insert(key, isOn);
}

void implementation::SettingsPage::selectComboBoxItem(const winrt::Microsoft::UI::Xaml::Controls::ComboBox& comboBox, const uint32_t& value)
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

uint32_t implementation::SettingsPage::getSelectedComboBoxItemTag(const winrt::Microsoft::UI::Xaml::Controls::ComboBox& comboBox)
{
    return std::stoi(comboBox.SelectedItem().as<xaml::ComboBoxItem>().Tag().as<hstring>().data());
}
