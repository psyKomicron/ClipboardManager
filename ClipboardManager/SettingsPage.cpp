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

namespace winrt
{
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Controls;
    using namespace winrt::Microsoft::UI::Xaml::Controls::Primitives;
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Storage::Pickers;
}
namespace impl = winrt::ClipboardManager::implementation;

void impl::SettingsPage::Page_Loading(winrt::FrameworkElement const&, winrt::IInspectable const&)
{
    clipmgr::utils::StartupTask startupTask{};
    AutoStartToggleSwitch().IsOn(startupTask.isTaskRegistered());

    SaveMatchingResultsToggleSwitch().IsOn(settings.get<bool>(L"SaveMatchingResults").value_or(false));
    StartMinimizedToggleSwitch().IsOn(settings.get<bool>(L"StartWindowMinimized").value_or(false));
    NotificationsToggleSwitch().IsOn(settings.get<bool>(L"NotificationsEnabled").value_or(false));

    /*settings.get<clipmgr::notifs::NotificationDuration>(L"NotificationDurationType");
    settings.get<clipmgr::notifs::NotificationScenario>(L"NotificationScenarioType");
    settings.get<clipmgr::notifs::NotificationSound>(L"NotificationSoundType");*/
    
    auto durationType = settings.get<clipmgr::notifs::NotificationDuration>(L"NotificationDurationType").value_or(clipmgr::notifs::NotificationDuration::Default);
    DurationDefaultToggleButton().IsChecked(durationType == clipmgr::notifs::NotificationDuration::Default);
    DurationShortToggleButton().IsChecked(durationType == clipmgr::notifs::NotificationDuration::Short);
    DurationLongToggleButton().IsChecked(durationType == clipmgr::notifs::NotificationDuration::Long);

    NotificationScenariosComboBox().SelectedIndex(settings.get<int32_t>(L"NotificationScenarioType").value_or(0));
    NotificationSoundComboBox().SelectedIndex(settings.get<int32_t>(L"NotificationSoundType").value_or(0));
}

void impl::SettingsPage::Page_Loaded(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    loaded = true;
}

void impl::SettingsPage::AutoStartToggleSwitch_Toggled(winrt::IInspectable const& s, winrt::RoutedEventArgs const&)
{
    check_loaded(loaded);
    auto sender = s.as<winrt::ToggleSwitch>();

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

void impl::SettingsPage::StartMinimizedToggleSwitch_Toggled(winrt::IInspectable const& s, winrt::RoutedEventArgs const&)
{
    check_loaded(loaded);
    updateSetting(s, L"StartWindowMinimized");
}

void impl::SettingsPage::SaveMatchingResultsToggleSwitch_Toggled(winrt::IInspectable const& s, winrt::RoutedEventArgs const&)
{
    check_loaded(loaded);
    updateSetting(s, L"SaveMatchingResults");
}

void impl::SettingsPage::EnableListeningToggleSwitch_Toggled(winrt::IInspectable const& s, winrt::RoutedEventArgs const&)
{
    NotificationsExpander().IsEnabled(NotificationsToggleSwitch().IsOn());
    
    check_loaded(loaded);
    updateSetting(s, L"NotificationsEnabled");
}

void impl::SettingsPage::DurationToggleButton_Click(winrt::IInspectable const& sender, winrt::RoutedEventArgs const& e)
{
    auto senderToggleButton = sender.as<winrt::ToggleButton>();
    if (senderToggleButton.IsChecked())
    {
        for (auto&& child : DurationButtonsStackPanel().Children())
        {
            auto toggleButton = child.as<winrt::ToggleButton>();
            if (toggleButton.Tag().as<hstring>() != senderToggleButton.Tag().as<hstring>())
            {
                toggleButton.IsChecked(false);
            }
        }
    }

    auto tag = std::stoi(std::wstring(senderToggleButton.Tag().as<hstring>()));
    settings.insert(L"NotificationDurationType", static_cast<clipmgr::notifs::NotificationDuration>(tag));
}

void impl::SettingsPage::NotificationScenariosComboBox_SelectionChanged(winrt::IInspectable const&, winrt::SelectionChangedEventArgs const& e)
{
    check_loaded(loaded);
    settings.insert(L"NotificationScenarioType", NotificationScenariosComboBox().SelectedItem().as<winrt::FrameworkElement>().Tag().as<int32_t>());
}

void impl::SettingsPage::NotificationSoundComboBox_SelectionChanged(winrt::IInspectable const&, winrt::SelectionChangedEventArgs const& e)
{
    check_loaded(loaded);
    settings.insert(L"NotificationSoundType", NotificationSoundComboBox().SelectedItem().as<winrt::FrameworkElement>().Tag().as<int32_t>());
}

void impl::SettingsPage::IgnoreCaseToggleSwitch_Toggled(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    check_loaded(loaded);
    settings.insert(L"RegexIgnoreCase", IgnoreCaseToggleSwitch().IsOn());
}

void impl::SettingsPage::RegexModeComboBox_SelectionChanged(winrt::IInspectable const&, winrt::SelectionChangedEventArgs const&)
{
    check_loaded(loaded);
    settings.insert(L"TriggerMatchMode", RegexModeComboBox().SelectedItem().as<winrt::FrameworkElement>().Tag().as<int32_t>());
}

winrt::async impl::SettingsPage::CreateExampleTriggersFileButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
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

winrt::async impl::SettingsPage::OverwriteExampleTriggersFileButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
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


void impl::SettingsPage::updateSetting(winrt::Windows::Foundation::IInspectable const& s, const std::wstring& key)
{
    auto sender = s.as<winrt::ToggleSwitch>();
    auto isOn = sender.IsOn();
    settings.insert(key, isOn);
}
