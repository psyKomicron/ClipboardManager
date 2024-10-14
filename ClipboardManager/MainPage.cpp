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
#include "ClipboardHistoryItemView.h"

#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.ApplicationModel.DataTransfer.h>
#include <winrt/Windows.ApplicationModel.Resources.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Microsoft.Windows.AppNotifications.h>
#include <winrt/Microsoft.Windows.AppNotifications.Builder.h>
#include <winrt/Windows.System.h>
// OCR
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.Globalization.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <src/utils/com_closable_ptr.h>

#include <boost/property_tree/xml_parser.hpp>

#include <Shobjidl.h>
#include <ppltasks.h>

#include <iostream>
#include <chrono>

using namespace std::chrono_literals;

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
                openSaveFileState,
                viewActionsState,
                noClipboardTriggersToDisplayState,
                firstStartupState,
                normalStartupState,
                quickSettingsClosedState,
                quickSettingsOpenState
            });

        manager.registerActionCallback([this](std::wstring args)
        {
            LaunchAction(args);
        });

        manager.registerActivatedCallback([this]()
        {
            clip::utils::getCurrentAppWindow().Show();
        });
    }

    MainPage::MainPage(const winrt::Microsoft::UI::Windowing::AppWindow& appWindow) : MainPage()
    {
        this->appWindow = appWindow;
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
        win::Clipboard::ContentChanged(clipboardContentChangedToken);

        auto userFilePath = localSettings.get<std::filesystem::path>(L"UserFilePath");

        if (userFilePath.has_value())
        {
            if (!triggers.empty())
            {
                clip::ClipboardTrigger::saveClipboardTriggers(triggers, userFilePath.value());
            }

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

    void MainPage::UpdateTitleBar()
    {
        auto&& presenter = appWindow.Presenter().try_as<xaml::OverlappedPresenter>();
        if (presenter)
        {
            WindowButtonsColumn().Width(xaml::GridLengthHelper::FromPixels(
                presenter.IsMinimizable() ? 135 : 45
            ));
        }
    }


    void MainPage::Page_SizeChanged(win::IInspectable const&, xaml::SizeChangedEventArgs const&)
    {
        std::vector<winrt::Windows::Graphics::RectInt32> dragRectangles{};

        winrt::Windows::Graphics::RectInt32 rect
        {
            // X
            static_cast<int32_t>(OverlayButtonColumn().ActualWidth()),
            // Y
            3,
            // Width
            static_cast<int32_t>(DragBorderColumn().ActualWidth()),
            // Height
            static_cast<int32_t>(TitleBarGrid().ActualHeight())
        };

        appWindow.TitleBar().SetDragRectangles(dragRectangles);
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

            clip::notifs::toasts::compat::DesktopNotificationManagerCompat::Uninstall();
        }

        localSettings.insert(L"CurrentAppVersion", APP_VERSION);

        if (localSettings.get<bool>(L"StartWindowMinimized").value_or(false))
        {
            appWindow.Presenter().as<xaml::OverlappedPresenter>().Minimize();
            // TODO: Send toast notification to tell the user the app has started ?
        }

        co_await LoadClipboardHistory();
    }

    winrt::async MainPage::Page_Loaded(win::IInspectable const&, xaml::RoutedEventArgs const&)
    {
        Restore();

        loaded = true;

        clipboardContentChangedToken = win::Clipboard::ContentChanged({ this, &MainPage::ClipboardContent_Changed });

        co_await resume_background();

        if (!localSettings.get<bool>(L"FirstStartup").has_value())
        {
            localSettings.insert(L"FirstStartup", false);

            DispatcherQueue().TryEnqueue([this]()
            {
                visualStateManager.goToState(firstStartupState);
            });
        }

        DispatcherQueue().TryEnqueue([this]()
        {
            visualStateManager.goToState(triggers.empty()
                                         ? noClipboardTriggersToDisplayState
                                         : displayClipboardTriggersState);
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

                editor.IsOn({ this, &MainPage::Editor_Toggled });
                editor.LabelChanged({ this, &MainPage::Editor_LabelChanged });
                editor.Changed({ this, &MainPage::Editor_Changed });

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

            visualStateManager.goToState(viewActionsState);
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

            visualStateManager.goToState(openSaveFileState);
            ReloadTriggers();
        }
    }

    void MainPage::StartTourButton_Click(win::IInspectable const&, xaml::RoutedEventArgs const&)
    {
        visualStateManager.goToState(normalStartupState);
        visualStateManager.goToState(quickSettingsOpenState);

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
                visualStateManager.goToState(displayClipboardTriggersState);

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
                visualStateManager.goToState(noClipboardTriggersToDisplayState);
            }

            Pivot().SelectedIndex(3);
        }
    }

    void MainPage::OpenQuickSettingsButton_Click(win::IInspectable const&, xaml::RoutedEventArgs const&)
    {
        visualStateManager.switchState(quickSettingsClosedState.group());
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
                MessagesBar().AddMessage(L"ErrorMessage_CannotSaveTriggersFileNotFound", L"Cannot save triggers, the specified user file cannot be found.");
            }
            catch (boost::property_tree::xml_parser_error xmlParserError)
            {
                MessagesBar().AddMessage(L"ErrorMessage_CannotSaveTriggersXmlError", L"Cannot save triggers, XML parsing error occured.");
            }
        }
        else
        {
            MessagesBar().AddMessage(L"ErrorMessage_CannotSaveTriggersNoUserFile", L"Cannot save triggers, no user file has been specified for this application.");
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
        auto add = co_await ClipboardTriggerEditControl().Edit();

        if (add)
        {
            auto label = std::wstring(ClipboardTriggerEditControl().TriggerLabel());
            auto format = std::wstring(ClipboardTriggerEditControl().TriggerFormat());
            auto ignoreCase = ClipboardTriggerEditControl().IgnoreCase();
            auto regex = boost::wregex(std::wstring(ClipboardTriggerEditControl().TriggerRegex()),
                                       (ignoreCase ? boost::regex_constants::icase : 0u));
            auto useSearch = ClipboardTriggerEditControl().UseSearch();

            auto trigger = clip::ClipboardTrigger(label, format, regex, true);
            trigger.updateMatchMode(useSearch ? clip::MatchMode::Search : clip::MatchMode::Match);
            triggers.push_back(trigger);

            auto view = make<ClipboardManager::implementation::ClipboardActionEditor>();
            view.ActionLabel(label);
            view.ActionFormat(format);
            view.IgnoreCase(ignoreCase);
            view.ActionRegex(regex.str());
            view.UseSearch(useSearch);

            ClipboardTriggersListView().Items().Append(view);
        }
    }


    winrt::async MainPage::ClipboardContent_Changed(const win::IInspectable&, const win::IInspectable&)
    {
        auto content = win::Clipboard::GetContent();

        if (content.Properties().ApplicationName() == L"ClipboardManager")
        {
            logger.info(L"Clipboard content changed, but the available format is not text or the application that changed the clipboard content is me.");
            co_return;
        }

        AddClipboardItem(content, true);
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
                    if (clipboardItem.IndexOf(pos, label))
                    {
                        clipboardItem.EditAction(pos, action.label(), action.format(), action.regex().str(), action.enabled());
                    }
                }

                break;
            }
        }
    }

    void MainPage::Editor_Changed(const winrt::ClipboardManager::ClipboardActionEditor& sender, const Windows::Foundation::IInspectable& inspectable)
    {
        auto actionLabel = std::wstring(sender.ActionLabel());
        auto format = std::wstring(sender.ActionFormat());
        auto newRegex = std::wstring(sender.ActionRegex());

        auto args = inspectable.as<int32_t>();
        bool ignoreCase = args & (1 << 1);
        bool useSearch = args & 1;
        auto flags = ignoreCase ? boost::regex_constants::icase : 0;

        for (auto&& action : triggers)
        {
            if (action.label() == actionLabel)
            {
                action.format(format);
                action.regex(boost::wregex(newRegex, flags));
                action.updateMatchMode(useSearch ? clip::MatchMode::Search : clip::MatchMode::Match);

                for (auto&& clipboardItem : clipboardActionViews)
                {
                    uint32_t pos = 0;
                    if (clipboardItem.IndexOf(pos, action.label()))
                    {
                        clipboardItem.EditAction(pos, action.label(), action.format(), action.regex().str(), action.enabled());
                    }
                }

                break;
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
                    if (view.IndexOf(index, action.label()))
                    {
                        view.IsEnabled(action.enabled());
                    }
                }

                action.enabled(isOn);
            }
        }
    }

    void MainPage::TestRegexButton_Click(win::IInspectable const&, xaml::RoutedEventArgs const&)
    {
        TestRegexContentDialog().Open();
    }

    void MainPage::OverlayToggleButton_Click(win::IInspectable const&, xaml::RoutedEventArgs const&)
    {
        auto&& presenter = appWindow.Presenter().as<xaml::OverlappedPresenter>();
        if (presenter)
        {
            bool toggled = OverlayToggleButton().IsChecked().GetBoolean();
            if (toggled)
            {
                presenter.IsMinimizable(!toggled);
                presenter.IsMaximizable(!toggled);

                visualStateManager.goToState(overlayWindowState);
            }
            else
            {
                presenter.IsMinimizable(localSettings.get<bool>(L"AllowWindowMinimize").value_or(true));
                presenter.IsMaximizable(localSettings.get<bool>(L"AllowWindowMaximize").value_or(false));

                visualStateManager.goToState(normalWindowState);
            }
            presenter.IsAlwaysOnTop(toggled);
        }
    }

    void MainPage::CommandBarImportButton_Click(win::IInspectable const&, xaml::RoutedEventArgs const&)
    {
        LocateUserFileButton_Click(nullptr, nullptr);
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

            auto&& presenter = appWindow.Presenter().try_as<xaml::OverlappedPresenter>();
            if (presenter)
            {
                if (presenter.State() != winrt::OverlappedPresenterState::Restored)
                {
                    presenter.Restore();
                }
                else
                {
                    presenter.Minimize();
                }
            }
        });

        auto&& presenter = appWindow.Presenter().try_as<xaml::OverlappedPresenter>();
        if (presenter)
        {
            WindowButtonsColumn().Width(xaml::GridLengthHelper::FromPixels(
                presenter.IsMinimizable() ? 135 : 45
            ));
        }
    }

    void MainPage::AddAction(const std::wstring& text, const bool& notify)
    {
        if (localSettings.get<bool>(L"AddDuplicatedActions").value_or(true)
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
                    uint32_t i = 0;
                    if (clipboardActionViews.IndexOf(sender, i))
                    {
                        clipboardActionViews.RemoveAt(i);
                    }
                });

                if (notify)
                {
                    SendNotification(buttons);
                }
            }
        }
    }

    bool MainPage::FindActions(const winrt::ClipboardManager::ClipboardActionView& actionView,
                               std::vector<std::pair<std::wstring, std::wstring>>& buttons, const std::wstring& text)
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

                try
                {
                    auto url = std::vformat(action.format(), std::make_wformat_args(text));
                    buttons.push_back({ action.label(), url });
                }
                catch (std::invalid_argument formatError)
                {
                    logger.error(clip::utils::convert(formatError.what()));

                    MessagesBar().AddWarning(L"", L"Failed to create format for trigger " + action.label());
                }
            }
        }

        return hasMatch;
    }

    void MainPage::SendNotification(const std::vector<std::pair<std::wstring, std::wstring>>& buttons)
    {
        if (!localSettings.get<bool>(L"NotificationsEnabled").value_or(true)
            || SilenceNotificationsToggleButton().IsChecked().GetBoolean())
        {
            return;
        }

        auto durationType = localSettings.get<clip::notifs::NotificationDurationType>(L"NotificationDurationType").value_or(clip::notifs::NotificationDurationType::Default);
        auto scenarioType = localSettings.get<clip::notifs::NotificationScenarioType>(L"NotificationScenarioType").value_or(clip::notifs::NotificationScenarioType::Default);
        auto soundType = localSettings.get<clip::notifs::NotificationSoundType>(L"NotificationSoundType").value_or(clip::notifs::NotificationSoundType::Default);

        clip::notifs::ToastNotification notif{};

        if (!notif.tryAddButtons(buttons))
        {
            notif.addText(std::wstring(
                resLoader.getOrAlt(L"ToastNotification_TooManyButtons",
                                   L"Too many triggers matched, open the app to activate the action of your choice")));

            notif.addButton(
                std::wstring(resLoader.getOrAlt(L"ToastNotification_OpenApp", L"Open app")),
                L"action=focus");
        }
        else
        {
            notif.addText(std::wstring(
                resLoader.getOrAlt(L"ToastNotification_ClickAction",
                                   L"Click corresponding trigger to activate it")));
        }

        try
        {
            notif.send(durationType, scenarioType, soundType);
        }
        catch (hresult_access_denied accessDenied)
        {
            MessagesBar().AddWarning(L"ToastNotification_FailedToSendWarning", L"Failed to send toast notification.");
        }
    }

    void MainPage::ReloadTriggers()
    {
        auto userFilePath = localSettings.get<std::filesystem::path>(L"UserFilePath");
        if (userFilePath.has_value()
            && clip::utils::pathExists(userFilePath.value())
            && LoadTriggers(userFilePath.value()))
        {
            try
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
            catch (...)
            {
                logger.error(L"Failed to reload triggers.");
            }
        }

        MessagesBar().AddError(L"ReloadActionsUserFileNotFoundTitle",
                               L"Failed to reload triggers",
                               L"ReloadActionsUserFileNotFoundMessage",
                               L"Actions user file has either been moved/deleted or application settings have been cleared.");
    }

    bool MainPage::LoadTriggers(std::filesystem::path& path)
    {
        try
        {
            triggers = clip::ClipboardTrigger::loadClipboardTriggers(path);

            bool showError = false;
            std::wstring invalidTriggers{};
            for (auto&& trigger : triggers)
            {
                try
                {
                    trigger.checkFormat();
                }
                catch (clip::ClipboardTriggerFormatException formatExcept)
                {
                    logger.error(std::format(L"Trigger '{}' has an invalid format.", trigger.label()));

                    showError = true;

                    invalidTriggers += L"\n" + trigger.label();
                }
            }

            if (showError)
            {
                MessagesBar().Add(L"Invalid trigger",
                                  L"One or more triggers have invalid data, check the triggers page for more information. Invalid triggers:" + invalidTriggers,
                                  xaml::InfoBarSeverity::Error);
            }
            else
            {
                visualStateManager.goToState(
                    triggers.empty()
                    ? noClipboardTriggersToDisplayState
                    : displayClipboardTriggersState);
            }

            return true;
        }
        catch (clip::ClipboardTriggerFormatException formatExcept)
        {
            MessagesBar().AddError(L"", L"One trigger doesn't respect the valid format syntax.");

            triggers.clear();
        }
        catch (std::invalid_argument invalidArgument)
        {
            MessagesBar().AddError(L"ErrorMessage_TriggersFileNotFound",
                                   L"Triggers file not available or contains invalid data.");
        }
        catch (boost::property_tree::xml_parser_error xmlParserError)
        {
            MessagesBar().AddError(L"ErrorMessage_XmlParserError",
                                   L"Triggers file has invalid XML markup data.");
        }
        catch (boost::property_tree::ptree_bad_path badPath)
        {
            hstring message = resLoader.getOrAlt(L"ErrorMessage_InvalidTriggersFile", L"Triggers file has invalid XML markup data.");
            hstring content = L"";

            auto wpath = badPath.path<boost::property_tree::wpath>();
            auto path = clip::utils::convert(wpath.dump());

            if (path == L"settings.triggers")
            {
                // Missing <settings><triggers> node.
                content = resLoader.getOrAlt(L"ErrorMessage_MissingTriggersNode",
                                             L"XML declaration is missing '<triggers>' node.\nCheck settings for an example of a valid XML declaration.");
            }
            else if (path == L"settings.actions")
            {
                // Old version of triggers file.
                content = resLoader.getOrAlt(L"ErrorMessage_XmlOldVersion",
                                             L"<actions> node has been renamed <triggers> and <action> <actions>. Rename those nodes in your XML file and reload triggers.\nYou can easily access your user file via settings and see an example of a valid XML declaration there.");
            }
            else
            {
                content = L"Path not found: '" + hstring(path) + L"'";
            }

            MessagesBar().Add(message, content, xaml::InfoBarSeverity::Error);
        }

        return false;
    }

    void MainPage::LaunchAction(const std::wstring& url)
    {
        concurrency::create_task([this, url]() -> void
        {
            try
            {
                clip::utils::Launcher launcher{};
                launcher.launch(url).get();
            }
            catch (hresult_error error)
            {
                logger.error(L"Failed to launch url: " + std::wstring(error.message().data()));

                // I18N: Failed to launch url: '{}'
                DispatcherQueue().TryEnqueue([this, url]()
                {
                    MessagesBar().AddError(L"", L"Failed to launch url: '" + url + L"'.");
                });
            }
        });
    }

    winrt::async MainPage::LoadClipboardHistory()
    {
        auto loadClipboardHistory = localSettings.get<bool>(L"ImportClipboardHistory").value_or(false);

        try
        {
            auto&& clipboardHistory = co_await win::Clipboard::GetHistoryItemsAsync();
            auto&& items = clipboardHistory.Items();
            for (int i = items.Size() - 1; i >= 0; i--)
            {
                auto&& item = items.GetAt(i);
                auto&& content = item.Content();
                co_await AddClipboardItem(content, loadClipboardHistory);

                throw hresult_error();
            }
        }
        catch (hresult_error error)
        {
            logger.error(L"Failed to retreive item from clipboard history.");

            MessagesBar().AddWarning(L"Warning_FailedToLoadClipboardHistory", L"Failed to load clipboard history.");
        }
    }

    Windows::Foundation::IAsyncOperation<Windows::Media::Ocr::OcrResult> MainPage::RunOcr(Windows::Storage::Streams::IRandomAccessStreamWithContentType& bitmapStream)
    {
        auto&& bitmapDecoder = co_await Windows::Graphics::Imaging::BitmapDecoder::CreateAsync(bitmapStream);
        auto&& softwareBitmap = co_await bitmapDecoder.GetSoftwareBitmapAsync();
        clip::utils::unique_closable softwareBitmapPtr{ softwareBitmap };

        auto ocrEngine = Windows::Media::Ocr::OcrEngine::TryCreateFromUserProfileLanguages();
        if (ocrEngine)
        {
            try
            {
                auto&& ocrResult = co_await ocrEngine.RecognizeAsync(softwareBitmap);
                co_return ocrResult;
            }
            catch (hresult_error error)
            {
                logger.error(error.message().data());
            }
        }
        else
        {
            logger.error(L"Failed to create OCR engine from user profile languages.");
        }

        co_return Windows::Media::Ocr::OcrResult(nullptr);
    }

    winrt::async MainPage::AddClipboardItem(Windows::ApplicationModel::DataTransfer::DataPackageView& content, const bool& runTriggers)
    {
        if (content.Contains(win::StandardDataFormats::Text()))
        {
            auto&& itemText = co_await content.GetTextAsync();

            DispatcherQueue().TryEnqueue([this, itemText, runTriggers]()
            {
                // Run triggers on the text:
                if (runTriggers)
                {
                    auto&& text = std::wstring(itemText);
                    AddAction(text, true);
                }

                // Add text/clipboard content to history list:
                auto view = make<ClipboardHistoryItemView>();
                view.HostContent(box_value(itemText));

                ClipboardHistoryListView().Items().InsertAt(0, view);
            });
        }
        else if (localSettings.get<bool>(L"EnableOCR").value_or(false) && content.Contains(win::StandardDataFormats::Bitmap()))
        {
            logger.debug(L"Unregistering app from clipboard changes.");
            win::Clipboard::ContentChanged(clipboardContentChangedToken);

            try
            {
                logger.info(L"*Detected image in clipboard, performing OCR on it*");

                // Get bitmap stream from DataPackageView:
                auto&& clipboardStream = co_await content.GetBitmapAsync();

                auto&& bitmapStream = co_await clipboardStream.OpenReadAsync();
                clip::utils::unique_closable<Windows::Storage::Streams::IRandomAccessStream> bitmapStreamCloser{ bitmapStream };

                auto&& ocrResult = co_await RunOcr(bitmapStream);
                logger.debug(L"OCR result: " + std::wstring(ocrResult.Text().data()));

                // If the OCR found text, run triggers on the text:
                if (runTriggers && !ocrResult.Text().empty())
                {
                    std::wstring string{};
                    for (auto&& line : ocrResult.Lines())
                    {
                        if (string.empty())
                        {
                            string += line.Text();
                        }
                        else
                        {
                            string += L"\n" + line.Text();
                        }
                    }

                    DispatcherQueue().TryEnqueue([this, string]()
                    {
                        AddAction(string, true);
                    });
                }

                try
                {
                    logger.debug(L"*Adding clipboard image to display*");
                    logger.debug(L"Re-opening clipboard stream");
                    content = win::Clipboard::GetContent();
                    auto&& clonedClipboardStream = co_await(content.GetBitmapAsync());

                    logger.debug(L"Clipboard bitmap stream reopened.");

                    // Create bitmap from bitmap stream and show it:
                    DispatcherQueue().TryEnqueue([this, bitmapStream = co_await(clonedClipboardStream.OpenReadAsync()), text = ocrResult.Text()]()
                    {
                        xaml::StackPanel panel{};
                        panel.Spacing(7);
                        panel.Orientation(xaml::Orientation::Vertical);

                        clip::utils::unique_closable<Windows::Storage::Streams::IRandomAccessStream> bitmapStreamCloser{ bitmapStream };

                        Microsoft::UI::Xaml::Media::Imaging::BitmapImage bitmapImage{};
                        bitmapImage.SetSourceAsync(bitmapStream);

                        xaml::Image image{};
                        image.Source(bitmapImage);
                        image.Height(200);

                        panel.Children().Append(image);

                        xaml::TextBlock textBlock{};
                        textBlock.TextWrapping(xaml::TextWrapping::Wrap);
                        textBlock.Text(text);

                        panel.Children().Append(textBlock);

                        auto view = make<ClipboardHistoryItemView>();
                        view.HostContent(panel);

                        logger.debug(L"Image panel created, displaying it.");

                        ClipboardHistoryListView().Items().InsertAt(0, view);
                    });
                }
                catch (winrt::hresult_error error)
                {
                    logger.error(L"Error while working on image stream: " + std::wstring(error.message()));
                }
            }
            catch (...)
            {
                logger.error(L"Unknown error while running OCR.");
            }

            clipboardContentChangedToken = win::Clipboard::ContentChanged({ this, &MainPage::ClipboardContent_Changed });
            logger.debug(L"Re-registered app for clipboard changes.");
        }
    }
}
