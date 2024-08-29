#include "pch.h"
#include "MainPage.h"
#if __has_include("MainPage.g.cpp")
#include "MainPage.g.cpp"
#endif

#include "ClipboardManager.h"
#include "Resource.h"
#include "src/Settings.hpp"
#include "src/utils/helpers.hpp"
#include "src/utils/Launcher.hpp"
#include "src/utils/AppVersion.hpp"
#include "src/utils/StartupTask.hpp"
#include "src/notifs/ToastNotification.hpp"
#include "src/notifs/win_toasts.hpp"
#include "src/notifs/NotificationTypes.hpp"

#include "ClipboardActionView.h"
#include "ClipboardActionEditor.h"

#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.ApplicationModel.DataTransfer.h>
#include <winrt/Windows.ApplicationModel.Resources.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Microsoft.Windows.AppNotifications.h>
#include <winrt/Microsoft.Windows.AppNotifications.Builder.h>
#include <winrt/Windows.System.h>

#include <boost/property_tree/xml_parser.hpp>

#include <Shobjidl.h>
#include <ppltasks.h>

#include <iostream>

namespace xaml
{
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Controls;
    using namespace winrt::Microsoft::UI::Windowing;
}

namespace win
{
    using namespace winrt::Microsoft::Windows::AppNotifications;
    using namespace winrt::Microsoft::Windows::AppNotifications::Builder;
    using namespace winrt::Windows::ApplicationModel;
    using namespace winrt::Windows::ApplicationModel::DataTransfer;
    using namespace winrt::Windows::ApplicationModel::Resources;
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Storage::Pickers;
    using namespace winrt::Windows::System;
}


namespace winrt::ClipboardManager::implementation
{
    MainPage::MainPage()
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

        manager.registerAction(L"action", [this](clip::notifs::ToastNotificationAction toastNotificationAction)
        {
            auto params = toastNotificationAction.parameters();
            auto&& action = toastNotificationAction.action<std::wstring>();
            auto&& url = params[L"url"];

            logger.debug(std::vformat(L"Action has been clicked: \n\t- {}\n\t- {}", std::make_wformat_args(action, url)));

            LaunchAction(url);
        });

        manager.registerAction(L"", [this](clip::notifs::ToastNotificationAction)
        {
            clip::utils::getCurrentAppWindow().Show();
        });
    }

    win::Collections::IObservableVector<winrt::ClipboardManager::ClipboardActionView> MainPage::Actions()
    {
        return clipboardActionViews;
    }

    void MainPage::Actions(const win::Collections::IObservableVector<winrt::ClipboardManager::ClipboardActionView>& value)
    {
        clipboardActionViews = value;
    }

    void MainPage::AppClosing()
    {
        auto userFilePath = localSettings.get<std::filesystem::path>(L"UserFilePath");

        if (userFilePath.has_value())
        {
            clip::ClipboardTrigger::saveClipboardTriggers(triggers, userFilePath.value());

            if (localSettings.get<bool>(L"SaveMatchingResults").value_or(false) && clipboardActionViews.Size() > 0)
            {
                boost::property_tree::wptree tree{};
                boost::property_tree::read_xml(userFilePath.value().string(), tree);

                auto historyNode = tree.get_child_optional(L"settings.history");
                if (historyNode.has_value())
                {
                    historyNode.value().clear();
                }

                for (int i = clipboardActionViews.Size() - 1; i >= 0; i--)
                {
                    auto&& view = clipboardActionViews.GetAt(i);
                    tree.add(L"settings.history.item", std::wstring(view.Text()));
                }

                boost::property_tree::write_xml(userFilePath.value().string(), tree);
            }
        }
    }


    winrt::async MainPage::Page_Loading(xaml::FrameworkElement const&, win::IInspectable const&)
    {
        loaded = false;

        clip::utils::AppVersion appVersion{ APP_VERSION };
        clip::utils::AppVersion currentAppVersion{ localSettings.get<std::wstring>(L"CurrentAppVersion").value_or(APP_VERSION) };
        if (appVersion.compare(currentAppVersion))
        {
            logger.info(L"Application has been updated, clearing settings and removing startup task.");

            localSettings.clear();

            clip::utils::StartupTask startupTask{};
            startupTask.remove();
        }

        // Enable notifications by default.
        if (!localSettings.get<bool>(L"NotificationsEnabled").has_value())
        {
            clip::utils::StartupTask startupTask{};
            startupTask.set();

            localSettings.insert(L"NotificationsEnabled", true);
        }

        localSettings.insert(L"CurrentAppVersion", APP_VERSION);

        if (localSettings.get<bool>(L"StartWindowMinimized").value_or(false))
        {
            auto&& appWindow = clip::utils::getCurrentAppWindow();
            appWindow.Presenter().as<xaml::OverlappedPresenter>().Minimize();
            // TODO: Send toast notification to tell the user the app has started ?
        }

        auto task = loadClipboardHistory();

        clipboardContentChangedrevoker = win::Clipboard::ContentChanged(winrt::auto_revoke, { this, &MainPage::ClipboardContent_Changed });

        co_await task;
    }

    winrt::async MainPage::Page_Loaded(win::IInspectable const&, xaml::RoutedEventArgs const&)
    {
        Restore();

        loaded = true;

        co_await resume_background();

        if (!localSettings.get<bool>(L"FirstStartup").has_value())
        {
            localSettings.insert(L"FirstStartup", false);

            DispatcherQueue().TryEnqueue([this]()
            {
                visualStateManager.goToState(FirstStartupState);
            });    
        }

        DispatcherQueue().TryEnqueue([this]()
        {
            visualStateManager.goToState(triggers.empty() 
                ? NoClipboardTriggersToDisplayState
                : DisplayClipboardTriggersState);
        });
    }

    void MainPage::ClipboadTriggersListPivot_Loaded(win::IInspectable const&, win::IInspectable const&)
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
                editor.IgnoreCase((action.regex().flags() & boost::regex_constants::icase) == boost::regex_constants::icase);
                editor.UseSearch(action.matchMode() == clip::MatchMode::Search);

                editor.FormatChanged({ this, &MainPage::Editor_FormatChanged });
                editor.LabelChanged({ this, &MainPage::Editor_LabelChanged });
                editor.IsOn({ this, &MainPage::Editor_Toggled });
                editor.RegexChanged({ this, &MainPage::Editor_RegexChanged });

                editor.Removed([=](auto&& sender, auto&&)
                {
                    auto&& label = editor.ActionLabel();
                    for (size_t i = 0; i < triggers.size(); i++)
                    {
                        if (action.label() == label)
                        {
                            triggers.erase(triggers.begin() + i);
                            break;
                            // TODO: Remove action buttons on any ClipboardActionView added.
                        }
                    }

                    for (auto&& item : ClipboardTriggersListView().Items())
                    {
                        auto view = item.try_as<ClipboardManager::ClipboardActionEditor>();
                        if (view && view.ActionLabel() == label)
                        {
                            uint32_t index = 0;
                            if (ClipboardTriggersListView().Items().IndexOf(item, index))
                            {
                                ClipboardTriggersListView().Items().RemoveAt(index);
                            }

                            break;
                        }
                    }
                });

                ClipboardTriggersListView().Items().Append(editor);
            }
        }
    }

    winrt::async MainPage::LocateUserFileButton_Click(win::IInspectable const&, xaml::RoutedEventArgs const&)
    {
        win::FileOpenPicker picker{};
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

    winrt::async MainPage::CreateUserFileButton_Click(win::IInspectable const&, xaml::RoutedEventArgs const&)
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
            clip::ClipboardTrigger::initializeSaveFile(userFilePath);

            visualStateManager.goToState(OpenSaveFileState);
            ReloadTriggers();
        }
    }

    void MainPage::StartTourButton_Click(win::IInspectable const&, xaml::RoutedEventArgs const&)
    {
        visualStateManager.goToState(NormalStartupState);
        visualStateManager.goToState(QuickSettingsOpenState);

        SettingsPivotTeachingTip().IsOpen(true);
    
        teachingTipIndex = 1;
    }

    winrt::async MainPage::TeachingTip_CloseButtonClick(xaml::TeachingTip const& sender, win::IInspectable const& args)
    {
        static std::vector<xaml::TeachingTip> teachingTips
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

                auto actionView = winrt::make<ClipboardActionView>(L"Clipboard content that matches a trigger");
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

    winrt::async MainPage::TeachingTip2_CloseButtonClick(xaml::TeachingTip const& sender, win::IInspectable const&)
    {
        static size_t index = 1;
        static std::vector<xaml::TeachingTip> teachingTips
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

                auto triggerView = make<ClipboardActionEditor>();
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

    void MainPage::OpenQuickSettingsButton_Click(win::IInspectable const&, xaml::RoutedEventArgs const&)
    {
        visualStateManager.switchState(QuickSettingsClosedState.group());
    }

    void MainPage::ReloadTriggersButton_Click(win::IInspectable const&, xaml::RoutedEventArgs const&)
    {
        ReloadTriggers();
    }

    void MainPage::CommandBarSaveButton_Click(win::IInspectable const&, xaml::RoutedEventArgs const&)
    {
        auto optPath = localSettings.get<std::filesystem::path>(L"UserFilePath");
        if (optPath.has_value())
        {
            try
            {
                clip::ClipboardTrigger::saveClipboardTriggers(triggers, optPath.value());
            }
            catch (std::invalid_argument invalidArgument)
            {
                auto message = clip::utils::getNamedResource(L"ErrorMessage_CannotSaveTriggersFileNotFound")
                    .value_or(L"Cannot save triggers, the specified user file cannot be found.");
            
                GenericErrorInfoBar().Message(message);
                GenericErrorInfoBar().IsOpen(true);
            }
            catch (boost::property_tree::xml_parser_error xmlParserError)
            {
                auto message = clip::utils::getNamedResource(L"ErrorMessage_CannotSaveTriggersXmlError")
                    .value_or(L"Cannot save triggers, XML parsing error occured.");

                GenericErrorInfoBar().Message(message);
                GenericErrorInfoBar().IsOpen(true);
            }
        }
        else
        {
            auto message = clip::utils::getNamedResource(L"ErrorMessage_CannotSaveTriggersNoUserFile")
                .value_or(L"Cannot save triggers, no user file has been specified for this application.");
            GenericErrorInfoBar().Message(message);
            GenericErrorInfoBar().IsOpen(true);
        }
    }

    void MainPage::ClearActionsButton_Click(win::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        clipboardActionViews.Clear();
    }

    winrt::async MainPage::ImportFromClipboardButton_Click(win::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        auto&& clipboardHistory = co_await win::Clipboard::GetHistoryItemsAsync();
        for (auto&& item : clipboardHistory.Items())
        {
            auto&& itemText = co_await item.Content().GetTextAsync();
        
            AddAction(itemText.data(), false);
        }
        // TODO: Notify user that clipboard content has been imported ?


    }

    winrt::async MainPage::AddTriggerButton_Click(win::IInspectable const&, xaml::RoutedEventArgs const&)
    {
        auto clipboardActionEditor = winrt::make<ClipboardActionEditor>();
        ClipboardTriggersListView().Items().Append(clipboardActionEditor);

        //co_await winrt::resume_background();

        auto add = co_await clipboardActionEditor.Edit();

        /*DispatcherQueue().TryEnqueue([this, add, clipboardActionEditor]()
        {
            if (!add)
            {
                uint32_t index = 0;
                if (ClipboardTriggersListView().Items().IndexOf(clipboardActionEditor, index))
                {
                    ClipboardTriggersListView().Items().RemoveAt(index);
                }
            }
            else
            {
                auto label = std::wstring(clipboardActionEditor.ActionLabel());
                auto format = std::wstring(clipboardActionEditor.ActionFormat());
                auto ignoreCase = clipboardActionEditor.IgnoreCase();
                auto regex = boost::wregex(std::wstring(clipboardActionEditor.ActionRegex()),
                    (ignoreCase ? boost::regex_constants::icase : 0u));
                auto useSearch = clipboardActionEditor.UseSearch();

                auto trigger = clip::ClipboardTrigger(label, format, regex, true);
                trigger.updateMatchMode(useSearch ? clip::MatchMode::Search : clip::MatchMode::Match);

                triggers.push_back(trigger);
            }
        });*/
    }


    winrt::async MainPage::ClipboardContent_Changed(const win::IInspectable&, const win::IInspectable&)
    {
        // TODO: There is a better way to add content to a usercontrol instead of creating it to potentially discard it.
        auto clipboardContent = win::Clipboard::GetContent();
        auto&& text = std::wstring(co_await clipboardContent.GetTextAsync());
        auto appName = clipboardContent.Properties().ApplicationName();
        if (!clipboardContent.Contains(win::StandardDataFormats::Text()) || appName == L"ClipboardManager")
        {
            logger.info(L"Clipboard content changed, but the available format is not text or the application that changed the clipboard content is me.");
            co_return;
        }

        AddAction(text, true);
    }

    void MainPage::Editor_LabelChanged(const winrt::ClipboardManager::ClipboardActionEditor& sender, const winrt::hstring& oldLabel)
    {
        auto label = std::wstring(oldLabel);
        auto newLabel = std::wstring(sender.ActionLabel());

        for (auto&& action : triggers)
        {
            if (action.label() == label)
            {
                action.label(newLabel);

                for (auto&& clipboardItem : clipboardActionViews)
                {
                    uint32_t pos = 0;
                    if (clipboardItem.IndexOf(pos, action.format(), label, action.regex().str(), action.enabled()))
                    {
                        clipboardItem.EditAction(pos, action.format(), action.label(), action.regex().str(), action.enabled());
                    }
                }

                break;
            }
        }
    }

    void MainPage::Editor_FormatChanged(const winrt::ClipboardManager::ClipboardActionEditor& sender, const winrt::hstring& oldFormat)
    {
        auto format = std::wstring(sender.ActionFormat());
        auto actionLabel = std::wstring(sender.ActionLabel());

        for (auto&& action : triggers)
        {
            if (action.label() == actionLabel)
            {
                action.format(format);

                for (auto&& clipboardItem : clipboardActionViews)
                {
                    uint32_t pos = 0;
                    if (clipboardItem.IndexOf(pos, oldFormat, action.label(), action.regex().str(), action.enabled()))
                    {
                        clipboardItem.EditAction(pos, action.format(), action.label(), action.regex().str(), action.enabled());
                    }
                }

                break;
            }
        }
    }

    void MainPage::Editor_RegexChanged(const winrt::ClipboardManager::ClipboardActionEditor& sender, const winrt::Windows::Foundation::IInspectable& inspectable)
    {
        auto args = inspectable.as<int32_t>();
        auto newRegex = std::wstring(sender.ActionRegex());
        auto actionLabel = std::wstring(sender.ActionLabel());
        for (auto&& action : triggers)
        {
            if (action.label() == actionLabel)
            {
                bool ignoreCase = args & (1 << 1);
                bool useSearch= args & 1;
                auto flags = ignoreCase ? boost::regex_constants::icase : 0;

                action.regex(boost::wregex(newRegex, flags));

                action.updateMatchMode(useSearch ? clip::MatchMode::Search : clip::MatchMode::Match);
            }
        }
    }

    void MainPage::Editor_Toggled(const winrt::ClipboardManager::ClipboardActionEditor& sender, const bool& isOn)
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

    void MainPage::TestRegexButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        TestRegexContentDialog().Open();
    }


    void MainPage::Restore()
    {
        // Load triggers:
        auto userFilePath = localSettings.get<std::filesystem::path>(L"UserFilePath");
        if (userFilePath.has_value() && clip::utils::pathExists(userFilePath.value()))
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
                logger.debug(L"Failed to retreive history from user file: " + clip::utils::convert(badPath.what()));
            }
        }

        // Get activation hot key (shortcut that brings the app window to the foreground):
        auto&& map = localSettings.get<std::map<std::wstring, clip::reg_types>>(L"ActivationHotKey");
        if (!map.empty())
        {
            auto key = std::get<std::wstring>(map[L"Key"])[0];
            auto mod = std::get<uint32_t>(map[L"Mod"]);
            activationHotKey = clip::HotKey(mod, key);
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

    void MainPage::AddAction(const std::wstring& text, const bool& notify)
    {
        bool addExisting = localSettings.get<bool>(L"AddDuplicatedActions").value_or(false);
        if (addExisting 
            || clipboardActionViews.Size() == 0
            || text != clipboardActionViews.GetAt(0).Text())
        {
            auto actionView = winrt::make<ClipboardActionView>(text.c_str());

            std::vector<std::pair<std::wstring, std::wstring>> buttons{};
            if (FindActions(actionView, buttons, text))
            {
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
            else
            {
                logger.info(L"No matching triggers found for '" + std::wstring(text) + L"'. Actions " + (triggers.empty() ? L"not loaded" : L"loaded"));
            }
        }
    }

    bool MainPage::FindActions(const winrt::ClipboardManager::ClipboardActionView& actionView, std::vector<std::pair<std::wstring, std::wstring>>& buttons, const std::wstring& text)
    {
        auto matchMode = localSettings.get<clip::MatchMode>(L"TriggerMatchMode");
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

    void MainPage::SendNotification(const std::vector<std::pair<std::wstring, std::wstring>>& buttons)
    {
        if (!localSettings.get<bool>(L"NotificationsEnabled").value_or(false))
        {
            logger.info(L"Not sending notification, notifications are not enabled.");
            return;
        }

        auto durationType = localSettings.get<clip::notifs::NotificationDurationType>(L"NotificationDurationType").value_or(clip::notifs::NotificationDurationType::Default);
        auto scenarioType = localSettings.get<clip::notifs::NotificationScenarioType>(L"NotificationScenarioType").value_or(clip::notifs::NotificationScenarioType::Default);
        auto soundType = localSettings.get<clip::notifs::NotificationSoundType>(L"NotificationSoundType").value_or(clip::notifs::NotificationSoundType::Default);

        clip::notifs::ToastNotification notif{};
        try
        {
            win::ResourceLoader resLoader{};
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

    void MainPage::ReloadTriggers()
    {
        auto userFilePath = localSettings.get<std::filesystem::path>(L"UserFilePath");
        if (userFilePath.has_value() && clip::utils::pathExists(userFilePath.value()) && LoadTriggers(userFilePath.value()))
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
        }
        else
        {
            xaml::InfoBar infoBar{};
            hstring title = L"Failed to reload triggers";
            hstring message = L"Actions user file has either been moved/deleted or application settings have been cleared.";
            try
            {
                win::ResourceLoader resLoader{};
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

    bool MainPage::LoadTriggers(std::filesystem::path& path)
    {
        try
        {
            triggers = clip::ClipboardTrigger::loadClipboardTriggers(path);

            visualStateManager.goToState(
                triggers.empty() 
                ? NoClipboardTriggersToDisplayState
                : DisplayClipboardTriggersState);

            return true;
        }
        catch (std::invalid_argument invalidArgument)
        {
            GenericErrorInfoBar().Message(
                clip::utils::getNamedResource(L"ErrorMessage_TriggersFileNotFound").value_or(L"Triggers file not found/available"));
            GenericErrorInfoBar().IsOpen(true);
        }
        catch (boost::property_tree::xml_parser_error xmlParserError)
        {
            GenericErrorInfoBar().Message(
                clip::utils::getNamedResource(L"ErrorMessage_XmlParserError").value_or(L"Triggers file has invalid markup data."));
            GenericErrorInfoBar().IsOpen(true);
        }
        catch (boost::property_tree::ptree_bad_path badPath)
        {
            hstring message = clip::utils::getNamedResource(L"ErrorMessage_InvalidTriggersFile").value_or(L"Triggers file has invalid markup data.");
            hstring content = L"";

            auto wpath = badPath.path<boost::property_tree::wpath>();
            auto path = clip::utils::convert(wpath.dump());

            if (path == L"settings.triggers")
            {
                // Missing <settings><triggers> node.
                content = clip::utils::getNamedResource(L"ErrorMessage_MissingTriggersNode")
                    .value_or(L"XML declaration is missing '<triggers>' node.\nCheck settings for an example of a valid XML declaration.");
            }
            else if (path == L"settings.actions")
            {
                // Old version of triggers file.
                content = clip::utils::getNamedResource(L"ErrorMessage_XmlOldVersion")
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

    void MainPage::LaunchAction(const std::wstring& url)
    {
        concurrency::create_task([this, url]() -> void
        {
            clip::utils::Launcher launcher{};
            launcher.launch(url).get();
        });
    }

    winrt::async MainPage::loadClipboardHistory()
    {
        auto&& clipboardHistory = co_await win::Clipboard::GetHistoryItemsAsync();
        for (auto&& item : clipboardHistory.Items())
        {
            try
            {
                uint32_t index = 0;
                auto&& availableFormats = item.Content().AvailableFormats();
                if (availableFormats.IndexOf(L"Text", index))
                {
                    auto&& itemText = co_await item.Content().GetTextAsync();
                    ClipboardHistoryListView().Items().Append(box_value(itemText));
                }
            }
            catch (hresult_error error)
            {
                logger.error(L"Failed to retreive item from clipboard history.");
            }
        }
    }
}
