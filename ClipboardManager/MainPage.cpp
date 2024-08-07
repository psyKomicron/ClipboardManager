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
#include "src/utils/Launcher.hpp"

#include "ClipboardActionView.h"
#include "ClipboardActionEditor.h"

#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.ApplicationModel.DataTransfer.h>
#include <winrt/Windows.ApplicationModel.Resources.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Microsoft.Windows.AppNotifications.h>
#include <winrt/Microsoft.Windows.AppNotifications.Builder.h>
#include <winrt/Windows.System.h>

#include <boost/property_tree/xml_parser.hpp>

#include <Shobjidl.h>
#include <ppltasks.h>
//#include <pplawait.h>

#include <iostream>

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
    using namespace winrt::Windows::System;
}

impl::MainPage::MainPage()
{
    visualStateManager.initializeStates(
    {
        OpenSaveFileState,
        ViewActionsState,
        NoClipboardTriggersToDisplayState,
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

        LaunchAction(url);
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

    auto&& clipboardHistory = co_await winrt::Clipboard::GetHistoryItemsAsync();
    for (auto&& item : clipboardHistory.Items())
    {
        auto&& itemText = co_await item.Content().GetTextAsync();
        ClipboardHistoryListView().Items().Append(box_value(itemText));
    }

    clipboardContentChangedrevoker = winrt::Clipboard::ContentChanged(winrt::auto_revoke, { this, &MainPage::ClipboardContent_Changed });
}

void impl::MainPage::Page_Loaded(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    Restore();

    loaded = true;

    /*presenter = clipmgr::utils::getCurrentAppWindow().Presenter().as<winrt::OverlappedPresenter>();
    presenter.Minimize();*/

    if (!localSettings.get<bool>(L"FirstStartup").has_value())
    {
        localSettings.insert(L"FirstStartup", false);
        visualStateManager.goToState(FirstStartupState);
    }
}

void impl::MainPage::ClipboadTriggersListPivot_Loaded(winrt::IInspectable const&, winrt::IInspectable const&)
{
    if (!triggers.empty())
    {
        logger.debug(std::format(L"Loading {} clipboard triggers.", triggers.size()));

        for (auto&& action : triggers)
        {
            auto editor = make<ClipboardManager::implementation::ClipboardActionEditor>();
            editor.ActionFormat(action.format());
            editor.ActionLabel(action.label());
            editor.ActionRegex(action.regex().str());
            editor.ActionEnabled(action.enabled());

            editor.FormatChanged({ this, &MainPage::Editor_FormatChanged });
            editor.LabelChanged({ this, &MainPage::Editor_LabelChanged });
            editor.IsOn({ this, &MainPage::Editor_Toggled });

            editor.Removed([=](auto&& sender, auto&& b)
            {
                auto&& label = editor.ActionLabel();
                for (size_t i = 0; i < triggers.size(); i++)
                {
                    if (action.label() == label)
                    {
                        // TODO: Remove action buttons on any ClipboardActionView added.
                        /*for (auto&& view : clipboardActionViews)
                        {
                        uint32_t index = 0;
                        if (view.IndexOf(index, action.format(), action.label(), action.regex().str(), action.enabled()))
                        {
                        }
                        }*/

                        triggers.erase(triggers.begin() + i);

                        break;
                    }
                }
            });

            ClipboardTriggersListView().Items().Append(editor);
        }
    }
}

void impl::MainPage::ClipboardActionsPivotItem_Loaded(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
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
        localSettings.insert(L"UserFilePath", userFilePath);

        ReloadTriggers();

        visualStateManager.goToState(ViewActionsState);
    }
}

winrt::async impl::MainPage::CreateUserFileButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
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

        visualStateManager.goToState(OpenSaveFileState);
        ReloadTriggers();
    }
}

void impl::MainPage::StartTourButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    visualStateManager.goToState(NormalStartupState);
    visualStateManager.goToState(QuickSettingsOpenState);

    SettingsPivotTeachingTip().IsOpen(true);
    
    teachingTipIndex = 1;
}

winrt::async impl::MainPage::TeachingTip_CloseButtonClick(winrt::TeachingTip const& sender, winrt::IInspectable const& args)
{
    static std::vector<winrt::TeachingTip> teachingTips
    {
        SettingsPivotTeachingTip(),
        ClipboardHistoryPivotTeachingTip(),
        ClipboardTriggersPivotTeachingTip(),
        ClipboardActionsPivotTeachingTip(),
        OpenQuickSettingsButtonTeachingTip(),
        ReloadActionsButtonTeachingTip(),
        SilenceNotificationsToggleButtonTeachingTip(),
        HistoryToggleButtonTeachingTip(),
        ListViewTeachingTip()
    };

    if (teachingTipIndex < teachingTips.size())
    {
        teachingTips[teachingTipIndex - 1].IsOpen(false);
        teachingTips[teachingTipIndex++].IsOpen(true);
    }
    else
    {
        teachingTips[teachingTipIndex - 1].IsOpen(false);

        // Start tour of triggers:
        bool manuallyAddedAction = false;
        if (clipboardActionViews.Size() == 0)
        {
            manuallyAddedAction = true;

            auto actionView = winrt::make<::impl::ClipboardActionView>(L"Clipboard content that matches a trigger");
            actionView.AddAction(L"{}", L"Trigger", L"", true);

            clipboardActionViews.InsertAt(0, actionView);
        }

        co_await clipboardActionViews.GetAt(0).StartTour();

        if (manuallyAddedAction)
        {
            clipboardActionViews.RemoveAt(0);
        }
        
        Pivot().SelectedIndex(2);
        CommandBarSaveButtonTeachingTip().IsOpen(true);
    }
}

winrt::async impl::MainPage::TeachingTip2_CloseButtonClick(winrt::TeachingTip const& sender, winrt::IInspectable const&)
{
    static size_t index = 1;
    static std::vector<winrt::TeachingTip> teachingTips
    {
        HistoryToggleButtonTeachingTip(),
        ClipboardTriggersListViewTeachingTip()
    };

    if (index < teachingTips.size())
    {
        teachingTips[index - 1].IsOpen(false);
        teachingTips[index++].IsOpen(true);
    }
    else
    {
        bool manuallyAddedAction = false;
        if (ClipboardTriggersListView().Items().Size() == 0)
        {
            visualStateManager.goToState(DisplayClipboardTriggersState);

            auto triggerView = make<::impl::ClipboardActionEditor>();
            triggerView.ActionLabel(L"Test trigger");
            triggerView.ActionFormat(L"https://duckduckgo.com/?q={}");
            triggerView.ActionRegex(L".+");
            triggerView.ActionEnabled(true);

            ClipboardTriggersListView().Items().InsertAt(0, triggerView);
            manuallyAddedAction = true;
        }

        co_await ClipboardTriggersListView().Items().GetAt(0).as<winrt::ClipboardManager::ClipboardActionEditor>().StartTour();

        if (manuallyAddedAction)
        {
            ClipboardTriggersListView().Items().RemoveAt(0);
            visualStateManager.goToState(NoClipboardTriggersToDisplayState);
        }

        Pivot().SelectedIndex(3);
    }
}

void impl::MainPage::OpenQuickSettingsButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    visualStateManager.switchState(QuickSettingsClosedState.group());
}

void impl::MainPage::ReloadTriggersButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    ReloadTriggers();
}

void impl::MainPage::CommandBarSaveButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    auto optPath = localSettings.get<std::filesystem::path>(L"UserFilePath");
    if (optPath.has_value())
    {
        try
        {
            clipmgr::ClipboardTrigger::saveClipboardTriggers(triggers, optPath.value());
        }
        catch (std::invalid_argument invalidArgument)
        {
            auto message = clipmgr::utils::getNamedResource(L"ErrorMessage_CannotSaveTriggersFileNotFound")
                .value_or(L"Cannot save triggers, the specified user file cannot be found.");
            
            GenericErrorInfoBar().Message(message);
            GenericErrorInfoBar().IsOpen(true);
        }
        catch (boost::property_tree::xml_parser_error xmlParserError)
        {
            auto message = clipmgr::utils::getNamedResource(L"ErrorMessage_CannotSaveTriggersXmlError")
                .value_or(L"Cannot save triggers, XML parsing error occured.");

            GenericErrorInfoBar().Message(message);
            GenericErrorInfoBar().IsOpen(true);
        }
    }
    else
    {
        auto message = clipmgr::utils::getNamedResource(L"ErrorMessage_CannotSaveTriggersNoUserFile")
            .value_or(L"Cannot save triggers, no user file has been specified for this application.");
        GenericErrorInfoBar().Message(message);
        GenericErrorInfoBar().IsOpen(true);
    }
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

    AddAction(text, true);
}

void impl::MainPage::Editor_FormatChanged(const winrt::ClipboardManager::ClipboardActionEditor& sender, const winrt::hstring& oldFormat)
{
    auto format = std::wstring(sender.ActionFormat());
    auto actionLabel = std::wstring(sender.ActionLabel());
    for (auto&& action : triggers)
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
    for (auto&& action : triggers)
    {
        if (action.label() == label)
        {
            logger.debug(std::format(L"Action label modified, old label : {} - new label : {}", label, newLabel));
            action.label(newLabel);

            for (auto&& clipboardItem : clipboardActionViews)
            {
                uint32_t pos = 0;
                if (clipboardItem.IndexOf(pos, action.format(), label, action.regex().str(), action.enabled()))
                {
                    clipboardItem.EditAction(pos, action.format(), action.label(), action.regex().str(), action.enabled());
                }
            }
        }
    }
}

void impl::MainPage::Editor_Toggled(const winrt::ClipboardManager::ClipboardActionEditor& sender, const bool& isOn)
{
    auto actionLabel = std::wstring(sender.ActionLabel());
    for (auto&& action : triggers)
    {
        if (action.label() == actionLabel)
        {
            for (auto&& view : clipboardActionViews)
            {
                uint32_t index = 0;
                if (view.IndexOf(index, action.format(), action.label(), action.regex().str(), action.enabled()))
                {
                    view.IsEnabled(action.enabled());
                }
            }

            action.enabled(isOn);
        }
    }
}


void impl::MainPage::Restore()
{
    // Load triggers:
    auto userFilePath = localSettings.get<std::filesystem::path>(L"UserFilePath");
    if (userFilePath.has_value() && clipmgr::utils::pathExists(userFilePath.value()))
    {
        LoadTriggers(userFilePath.value());

        try
        {
            boost::property_tree::wptree tree{};
            boost::property_tree::read_xml(userFilePath.value().string(), tree);
            for (auto&& historyItem : tree.get_child(L"settings.history"))
            {
                AddAction(historyItem.second.data(), false);
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
            logger.info(L"No matching triggers found for '" + std::wstring(text) + L"'. Actions " + (triggers.empty() ? L"not loaded" : L"loaded"));
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

bool impl::MainPage::FindActions(const winrt::ClipboardManager::ClipboardActionView& actionView, std::vector<std::pair<std::wstring, std::wstring>>& buttons, const std::wstring& text)
{
    auto matchMode = localSettings.get<clipmgr::MatchMode>(L"TriggerMatchMode");
    bool hasMatch = false;
    for (auto&& action : triggers)
    {
        if (action.enabled() && action.match(text, matchMode))
        {
            hasMatch = true;

            // TODO: When i add triggers, only enabled triggers will be added yet they can be enabled or disabled later. What do if.
            actionView.AddAction(action.format(), action.label(), action.regex().str(), true);

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

    auto durationType = localSettings.get<clipmgr::notifs::NotificationDurationType>(L"NotificationDurationType").value_or(clipmgr::notifs::NotificationDurationType::Default);
    auto scenarioType = localSettings.get<clipmgr::notifs::NotificationScenarioType>(L"NotificationScenarioType").value_or(clipmgr::notifs::NotificationScenarioType::Default);
    auto soundType = localSettings.get<clipmgr::notifs::NotificationSoundType>(L"NotificationSoundType").value_or(clipmgr::notifs::NotificationSoundType::Default);

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
                notif.addButton(L"Too many triggers matched, open the app to activate the action of your choice", L"action=focus");
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

void impl::MainPage::ReloadTriggers()
{
    auto userFilePath = localSettings.get<std::filesystem::path>(L"UserFilePath");
    if (userFilePath.has_value() && clipmgr::utils::pathExists(userFilePath.value()) && LoadTriggers(userFilePath.value()))
    {
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

        ClipboardTriggersListView().Items().Clear();
        ClipboadTriggersListPivot_Loaded(nullptr, nullptr);

        return;
    }
    else
    {
        winrt::InfoBar infoBar{};
        winrt::hstring title = L"Failed to reload triggers";
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

bool impl::MainPage::LoadTriggers(std::filesystem::path& path)
{
    try
    {
        triggers = clipmgr::ClipboardTrigger::loadClipboardTriggers(path);

        visualStateManager.goToState(
            triggers.empty() 
            ? NoClipboardTriggersToDisplayState
            : DisplayClipboardTriggersState);

        return true;
    }
    catch (std::invalid_argument invalidArgument)
    {
        GenericErrorInfoBar().Message(
            clipmgr::utils::getNamedResource(L"ErrorMessage_TriggersFileNotFound").value_or(L"Triggers file not found/available"));
        GenericErrorInfoBar().IsOpen(true);
    }
    catch (boost::property_tree::xml_parser_error xmlParserError)
    {
        GenericErrorInfoBar().Message(
            clipmgr::utils::getNamedResource(L"ErrorMessage_XmlParserError").value_or(L"Triggers file has invalid markup data."));
        GenericErrorInfoBar().IsOpen(true);
    }
    catch (boost::property_tree::ptree_bad_path badPath)
    {
        hstring message = clipmgr::utils::getNamedResource(L"ErrorMessage_InvalidTriggersFile").value_or(L"Triggers file has invalid markup data.");
        hstring content = L"";

        auto wpath = badPath.path<boost::property_tree::wpath>();
        auto path = clipmgr::utils::convert(wpath.dump());

        if (path == L"settings.triggers")
        {
            // Missing <settings><triggers> node.
            content = clipmgr::utils::getNamedResource(L"ErrorMessage_MissingTriggersNode")
                .value_or(L"XML declaration is missing '<triggers>' node.\nCheck settings for an example of a valid XML declaration.");
        }
        else if (path == L"settings.actions")
        {
            // Old version of triggers file.
            content = clipmgr::utils::getNamedResource(L"ErrorMessage_XmlOldVersion")
                .value_or(L"<actions> node has been renamed <triggers> and <action> <actions>. Rename those nodes in your XML file and reload triggers.\nYou can easily access your user file via settings and see an example of a valid XML declaration there.");
        }
        else
        {
            content = L"Path not found: '" + hstring(path) + L"'";
        }

        GenericErrorInfoBar().Title(message);
        GenericErrorInfoBar().Message(content);
        GenericErrorInfoBar().IsOpen(true);
    }

    return false;
}

void impl::MainPage::LaunchAction(const std::wstring& url)
{
    concurrency::create_task([this, url]() -> void
    {
        clipmgr::utils::Launcher launcher{};
        launcher.launch(url).get();
        /*if (!winrt::Launcher::LaunchUriAsync(winrt::Uri(url)).get())
        {
        logger.error(L"Failed to launch: " + url);
        }*/
    });
}
