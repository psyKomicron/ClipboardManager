#include "pch.h"
#include "MainPage.h"
#if __has_include("MainPage.g.cpp")
#include "MainPage.g.cpp"
#endif

#include "ClipboardManager.h"
#include "src/Settings.hpp"
#include "src/utils/helpers.hpp"
#include "src/notifs/ToastNotification.hpp"
#include "src/notifs/win_toasts.hpp"
#include "src/notifs/NotificationTypes.hpp"

#include "ClipboardActionView.h"
#include "ClipboardActionEditor.h"

#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.ApplicationModel.DataTransfer.h>
#include <winrt/Windows.ApplicationModel.Resources.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Microsoft.Windows.AppNotifications.h>
#include <winrt/Microsoft.Windows.AppNotifications.Builder.h>

#include <boost/property_tree/xml_parser.hpp>

#include <Shobjidl.h>

#include <iostream>
#include <filesystem>

namespace impl = winrt::ClipboardManager::implementation;
namespace winrt
{
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Controls;
    using namespace winrt::Microsoft::UI::Windowing;
    using namespace winrt::Microsoft::Windows::AppNotifications;
    using namespace winrt::Microsoft::Windows::AppNotifications::Builder;
    using namespace winrt::Windows::ApplicationModel;
    using namespace winrt::Windows::ApplicationModel::DataTransfer;
    using namespace winrt::Windows::ApplicationModel::Resources;
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Storage::Pickers;
}

impl::MainPage::MainPage()
{
    visualStateManager.initializeStates(
    {
        NormalActionsState,
        NoClipboardActionsState,
        OpenSaveFileState,
        ViewActionsState,
        NoClipboardActionsToDisplayState,
        FirstStartupState,
        NormalStartupState,
        QuickSettingsClosedState,
        QuickSettingsOpenState
    });

    manager.registerAction(L"action", [this](clipmgr::notifs::ToastNotificationAction toastNotificationAction)
    {
        auto params = toastNotificationAction.parameters();
        auto&& action = toastNotificationAction.action<std::wstring>();
        auto&& url = params[L"url"];
        logger.debug(std::vformat(L"Action has been clicked: \n\t- {}\n\t- {}", std::make_wformat_args(action, url)));
    });

    manager.registerAction(L"", [this](clipmgr::notifs::ToastNotificationAction)
    {
        clipmgr::utils::getCurrentAppWindow().Show();
    });
}

winrt::Windows::Foundation::Collections::IObservableVector<winrt::ClipboardManager::ClipboardActionView> winrt::ClipboardManager::implementation::MainPage::Actions()
{
    return clipboardActionViews;
}

void winrt::ClipboardManager::implementation::MainPage::Actions(const winrt::Windows::Foundation::Collections::IObservableVector<winrt::ClipboardManager::ClipboardActionView>& value)
{
    clipboardActionViews = value;
}

void winrt::ClipboardManager::implementation::MainPage::AppClosing()
{
    if (localSettings.get<bool>(L"SaveMatchingResults").value_or(false))
    {
        auto userFilePath = localSettings.get<std::filesystem::path>(L"UserFilePath");
        if (userFilePath.has_value())
        {
            if (clipboardActionViews.Size() == 0)
            {
                logger.debug(L"No history to save.");
                return;
            }

            boost::property_tree::wptree tree{};
            boost::property_tree::read_xml(userFilePath.value().string(), tree);
            tree.erase(L"settings.history");

            for (auto&& view : clipboardActionViews)
            {
                tree.add(L"settings.history.item", std::wstring(view.Text()));
            }

            boost::property_tree::write_xml(userFilePath.value().string(), tree);
        }
        else
        {
            logger.debug(L"User file path not saved in settings, impossible to save history.");
        }
    }
}


winrt::Windows::Foundation::IAsyncAction impl::MainPage::Page_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const&, winrt::Windows::Foundation::IInspectable const&)
{
    loaded = false;
    Restore();

    auto&& clipboardHistory = co_await winrt::Clipboard::GetHistoryItemsAsync();
    for (auto&& item : clipboardHistory.Items())
    {
        auto&& itemText = co_await item.Content().GetTextAsync();
        ClipboardHistoryListView().Items().Append(box_value(itemText));
    }

    clipboardContentChangedrevoker = winrt::Clipboard::ContentChanged(winrt::auto_revoke, { this, &MainPage::ClipboardContent_Changed });
}

void impl::MainPage::ClipboadActionsListPivot_Loaded(winrt::IInspectable const&, winrt::IInspectable const&)
{
    if (actions.empty())
    {
        visualStateManager.goToState(NoClipboardActionsToDisplayState);
        logger.debug(L"ClipboadActionsListPivot_Loaded > No clipboard actions to load.");
    }
    else
    {
        logger.debug(std::format(L"ClipboadActionsListPivot_Loaded > Loading {} clipboard actions.", actions.size()));

        for (auto&& action : actions)
        {
            auto editor = make<ClipboardManager::implementation::ClipboardActionEditor>();
            editor.ActionFormat(action.format());
            editor.ActionLabel(action.label());
            editor.ActionRegex(action.regex().str());
            editor.ActionEnabled(action.enabled());

            editor.FormatChanged({ this, &MainPage::Editor_FormatChanged });
            editor.LabelChanged({ this, &MainPage::Editor_LabelChanged });
            editor.IsOn({ this, &MainPage::Editor_Toggled });

            ClipboardActionsListView().Items().Append(editor);
        }
    }
}

void impl::MainPage::PivotItem_Loaded(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    if (actions.empty())
    {
        logger.info(L"'UserFilePath' doesn't exist or the path of 'UserFilePath' doesn't exist.");
        // Show info bar to create or locate user file.
        visualStateManager.goToState(NoClipboardActionsState);
    }
}

void impl::MainPage::Page_Loaded(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    loaded = true;

    /*presenter = clipmgr::utils::getCurrentAppWindow().Presenter().as<winrt::OverlappedPresenter>();
    presenter.Minimize();*/

    if (!localSettings.get<bool>(L"FirstStartup").has_value())
    {
        localSettings.insert(L"FirstStartup", false);
        visualStateManager.goToState(FirstStartupState);
    }
}

winrt::async impl::MainPage::LocateUserFileButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    winrt::FileOpenPicker picker{};
    picker.as<IInitializeWithWindow>()->Initialize(GetActiveWindow());
    picker.FileTypeFilter().Append(L".xml");
    
    auto&& storageFile = co_await picker.PickSingleFileAsync();
    if (storageFile)
    {
        std::filesystem::path userFilePath{ storageFile.Path().c_str() };

        clipmgr::Settings settings{};
        settings.insert(L"UserFilePath", userFilePath.wstring());

        visualStateManager.goToState(ViewActionsState);
    }
}

winrt::async impl::MainPage::CreateUserFileButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    winrt::FileSavePicker picker{};
    picker.as<IInitializeWithWindow>()->Initialize(GetActiveWindow());
    picker.SuggestedStartLocation(winrt::PickerLocationId::Unspecified);
    picker.FileTypeChoices().Insert(
        L"XML Files", single_threaded_vector<winrt::hstring>({ L".xml" })
    );
    picker.SuggestedFileName(L"user_file.xml");

    auto&& storageFile = co_await picker.PickSaveFileAsync();
    if (storageFile)
    {
        std::filesystem::path userFilePath{ storageFile.Path().c_str() };
        clipmgr::ClipboardAction::initializeSaveFile(userFilePath);

        visualStateManager.goToState(OpenSaveFileState);
    }
}

void impl::MainPage::ViewActionsButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    Pivot().SelectedIndex(2);
}

void impl::MainPage::StartTourButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    visualStateManager.goToState(NormalStartupState);
    visualStateManager.goToState(QuickSettingsOpenState);
    ListViewTeachingTip().IsOpen(true);
    teachingTipIndex = 1;
}

winrt::async impl::MainPage::TeachingTip_CloseButtonClick(winrt::TeachingTip const& sender, winrt::IInspectable const& args)
{
    static std::vector<winrt::TeachingTip> teachingTips
    {
        ListViewTeachingTip(),
        SettingsPivotTeachingTip(),
        ClipboardHistoryPivotTeachingTip(),
        ClipboardTriggersPivotTeachingTip(),
        ClipboardActionsPivotTeachingTip(),
        OpenQuickSettingsButtonTeachingTip(),
        ReloadActionsButtonTeachingTip(),
        SilenceNotificationsToggleButtonTeachingTip(),
        HistoryToggleButtonTeachingTip()
    };

    if (teachingTipIndex < teachingTips.size())
    {
        teachingTips[teachingTipIndex - 1].IsOpen(false);
        teachingTips[teachingTipIndex++].IsOpen(true);
    }
    else
    {
        teachingTips[teachingTipIndex - 1].IsOpen(false);

        // Start tour of actions:
        bool manuallyAddedAction = false;
        if (clipboardActionViews.Size() == 0)
        {
            manuallyAddedAction = true;

            auto actionView = winrt::make<::impl::ClipboardActionView>(L"Clipboard content that matches a trigger");
            actionView.AddAction(L"{}", L"Trigger", L"", true);
            actionView.Removed([this](auto&& sender, auto&&)
            {
                for (uint32_t i = 0; i < clipboardActionViews.Size(); i++)
                {
                    if (sender == clipboardActionViews.GetAt(i))
                    {
                        clipboardActionViews.RemoveAt(i);
                    }
                }
            });

            clipboardActionViews.InsertAt(0, actionView);
        }

        co_await clipboardActionViews.GetAt(0).StartTour();

        if (manuallyAddedAction)
        {
            clipboardActionViews.RemoveAt(0);
        }
        
        Pivot().SelectedIndex(2);

        manuallyAddedAction = false;
        if (ClipboardActionsListView().Items().Size() == 0)
        {
            // TODO: Manually add action.
        }

        ClipboardActionsListViewTeachingTip().IsOpen(true);

        if (manuallyAddedAction)
        {
            ClipboardActionsListView().Items().RemoveAt(0);
        }
    }
}

winrt::async impl::MainPage::TeachingTip2_CloseButtonClick(winrt::TeachingTip const& sender, winrt::IInspectable const&)
{
    static size_t index = 1;
    static std::vector<winrt::TeachingTip> teachingTips
    {
        HistoryToggleButtonTeachingTip(),
        CommandBarSaveButtonTeachingTip()
    };

    if (index < teachingTips.size())
    {
        teachingTips[index - 1].IsOpen(false);
        teachingTips[index++].IsOpen(true);
    }
    else
    {
        co_await ClipboardActionsListView().Items().GetAt(0).as<winrt::ClipboardManager::ClipboardActionEditor>().StartTour();
        Pivot().SelectedIndex(3);
    }
}

void impl::MainPage::OpenQuickSettingsButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    visualStateManager.switchState(QuickSettingsClosedState.group());
}

void impl::MainPage::ReloadActionsButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    auto userFilePath = localSettings.get<std::filesystem::path>(L"UserFilePath");
    if (userFilePath.has_value() && clipmgr::utils::pathExists(userFilePath.value()))
    {
        actions = clipmgr::ClipboardAction::loadClipboardActions(userFilePath.value());

        auto vector = winrt::single_threaded_observable_vector<winrt::hstring>();
        for (auto&& view : clipboardActionViews)
        {
            vector.Append(view.Text());
        }

        clipboardActionViews.Clear();

        for (auto&& view : vector)
        {
            AddAction(view.data(), false);
        }

        ClipboardActionsListView().Items().Clear();
        ClipboadActionsListPivot_Loaded(nullptr, nullptr);
    }
    else
    {
        winrt::InfoBar infoBar{};

        winrt::hstring title = L"Failed to reload actions";
        winrt::hstring message = L"Actions user file has either been moved/deleted or application settings have been cleared.";
        try
        {
            winrt::ResourceLoader resLoader{};
            title = resLoader.GetString(L"ReloadActionsUserFileNotFoundTitle");
            message = resLoader.GetString(L"ReloadActionsUserFileNotFoundMessage");
        }
        catch (winrt::hresult_error err)
        {
            logger.error(L"Failed to load resources file (resources.pri).");
        }

        infoBar.Title(title);
        infoBar.Message(message);
    }
}

void impl::MainPage::CommandBarSaveButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{

}


winrt::async impl::MainPage::ClipboardContent_Changed(const winrt::IInspectable&, const winrt::IInspectable&)
{
    // TODO: There is a better way to add content to a usercontrol instead of creating it to potentially discard it.
    auto clipboardContent = winrt::Clipboard::GetContent();
    auto&& text = std::wstring(co_await clipboardContent.GetTextAsync());
    auto appName = clipboardContent.Properties().ApplicationName();
    if (!clipboardContent.Contains(winrt::StandardDataFormats::Text()) || appName == L"ClipboardManager")
    {
        logger.info(L"Clipboard content changed, but the available format is not text or the application that changed the clipboard content is me.");
        co_return;
    }

    logger.debug(L"Clipboard content changed, application name: " + std::wstring(appName));

    AddAction(text);
}

void impl::MainPage::Editor_FormatChanged(const winrt::ClipboardManager::ClipboardActionEditor& sender, const winrt::hstring& oldFormat)
{
    auto format = std::wstring(sender.ActionFormat());
    auto actionLabel = std::wstring(sender.ActionLabel());
    for (auto&& action : actions)
    {
        if (action.label() == actionLabel)
        {
            action.format(format);
        }
    }
}

void impl::MainPage::Editor_LabelChanged(const winrt::ClipboardManager::ClipboardActionEditor& sender, const winrt::hstring& oldLabel)
{
    auto label = std::wstring(oldLabel);
    auto newLabel = std::wstring(sender.ActionLabel());
    for (auto&& action : actions)
    {
        if (action.label() == label)
        {
            logger.debug(std::format(L"Action label modified, old label : {} - new label : {}", label, newLabel));
            action.label(newLabel);
        }
    }

    /*for (auto&& clipboardItem : Actions())
    {
        if (clipboardItem.Action)
    }*/
}

void impl::MainPage::Editor_Toggled(const winrt::ClipboardManager::ClipboardActionEditor& sender, const bool& isOn)
{
    auto actionLabel = std::wstring(sender.ActionLabel());
    for (auto&& action : actions)
    {
        if (action.label() == actionLabel)
        {
            action.enabled(isOn);
        }
    }
}

void impl::MainPage::Restore()
{
    // Load actions:
    auto userFilePath = localSettings.get<std::filesystem::path>(L"UserFilePath");
    if (userFilePath.has_value() && clipmgr::utils::pathExists(userFilePath.value()))
    {
        actions = clipmgr::ClipboardAction::loadClipboardActions(userFilePath.value());

        try
        {
            boost::property_tree::wptree tree{};
            boost::property_tree::read_xml(userFilePath.value().string(), tree);

            for (auto&& historyItem : tree.get_child(L"settings.history"))
            {
                AddAction(historyItem.second.data());
            }
        }
        catch (const boost::property_tree::ptree_bad_path badPath)
        {
            logger.debug(L"Failed to retreive history from user file, somehow ptree_bad_path was thrown.");
        }
    }

    // Get activation hot key (shortcut that brings the app window to the foreground):
    auto&& map = localSettings.get<std::map<std::wstring, clipmgr::reg_types>>(L"ActivationHotKey");
    if (!map.empty())
    {
        auto key = std::get<std::wstring>(map[L"Key"])[0];
        auto mod = std::get<uint32_t>(map[L"Mod"]);
        //activationHotKey.~HotKey();
        activationHotKey = clipmgr::HotKey(mod, key);
    }

    activationHotKey.startListening([this]()
    {
        logger.debug(L"Hot key fired.");

        if (presenter.State() != winrt::OverlappedPresenterState::Restored)
        {
            presenter.Restore();
        }
        else
        {
            presenter.Minimize();
        }
    });
}

void impl::MainPage::AddAction(const std::wstring& text, const bool& notify)
{
    bool addExisting = localSettings.get<bool>(L"AddDuplicatedActions").value_or(false);
    if (addExisting 
        || clipboardActionViews.Size() == 0
        || text != clipboardActionViews.GetAt(0).Text())
    {
        auto actionView = winrt::make<::impl::ClipboardActionView>(text.c_str());

        std::vector<std::pair<std::wstring, std::wstring>> buttons{};
        if (!FindActions(actionView, buttons, text))
        {
            logger.info(L"No matching actions found for '" + std::wstring(text) + L"'. Actions " + (actions.empty() ? L"not loaded" : L"loaded"));
            return;
        }

        clipboardActionViews.InsertAt(0, actionView);
        actionView.Removed([this](auto&& sender, auto&&)
        {
            for (uint32_t i = 0; i < clipboardActionViews.Size(); i++)
            {
                if (sender == clipboardActionViews.GetAt(i))
                {
                    clipboardActionViews.RemoveAt(i);
                }
            }
        });

        if (notify)
        {
            SendNotification(buttons);
        }
    }
}

bool impl::MainPage::FindActions(const winrt::ClipboardManager::ClipboardActionView& actionView, 
    std::vector<std::pair<std::wstring, std::wstring>>& buttons, const std::wstring& text)
{
    bool hasMatch = false;
    for (auto&& action : actions)
    {
        if (action.enabled() && action.match(text))
        {
            hasMatch = true;

            // TODO: When i add actions, only enabled actions will be added yet they can be enabled or disabled later.
            actionView.AddAction(action.format(), action.label(), L"", true);

            auto url = std::vformat(action.format(), std::make_wformat_args(text));
            buttons.push_back({ action.label(), std::format(L"open&url={}", url) });
        }
    }

    return hasMatch;
}

void impl::MainPage::SendNotification(const std::vector<std::pair<std::wstring, std::wstring>>& buttons)
{
    if (!localSettings.get<bool>(L"NotificationsEnabled").value_or(false))
    {
        logger.info(L"Not sending notification, notifications are not enabled.");
        return;
    }

    auto durationType = localSettings.get<clipmgr::notifs::NotificationDuration>(L"NotificationDurationType").value_or(clipmgr::notifs::NotificationDuration::Default);
    auto scenarioType = localSettings.get<clipmgr::notifs::NotificationScenario>(L"NotificationScenarioType").value_or(clipmgr::notifs::NotificationScenario::Default);
    auto soundType = localSettings.get<clipmgr::notifs::NotificationSound>(L"NotificationSoundType").value_or(clipmgr::notifs::NotificationSound::Default);

    clipmgr::notifs::ToastNotification notif{};
    try
    {
        winrt::ResourceLoader resLoader{};
        if (!notif.tryAddButtons(buttons))
        {
            notif.addText(std::wstring(resLoader.GetString(L"ToastNotification_TooManyButtons")));
            notif.addButton(std::wstring(resLoader.GetString(L"ToastNotification_OpenApp")), L"action=focus");
        }
        else
        {
            notif.addText(std::wstring(resLoader.GetString(L"ToastNotification_ClickAction")));
        }

        notif.send(durationType, scenarioType, soundType);
    }
    catch (winrt::hresult_error err)
    {
        if (err.code() == 0x80070002)
        {
            // Don't forget to rename "ClipboardManager.pri" to "resources.pri".
            logger.error(L"Failed to load translation file (resources.pri), sending notification without translating the string.");
            if (!notif.tryAddButtons(buttons))
            {
                notif.addText(L"Open app");
                notif.addButton(L"Too many actions matched, open the app to activate the action of your choice", L"action=focus");
            }
            else
            {
                notif.addText(L"Activate action");
            }

            notif.addText(L"Text isn't translated due to app error.");
            notif.send(durationType, scenarioType, soundType);
        }
        else
        {
            logger.error(L"Failed to send toast notification. " + std::wstring(err.message()));
        }
    }
}
