#include "pch.h"
#include "MainPage.h"
#if __has_include("MainPage.g.cpp")
#include "MainPage.g.cpp"
#endif

// Application logic :
#include "ClipboardManager.h"
#include "Resource.h"
#include "lib/Settings.hpp"
#include "lib/utils/helpers.hpp"
#include "lib/utils/Launcher.hpp"
#include "lib/utils/AppVersion.hpp"
#include "lib/utils/StartupTask.hpp"
#include "lib/utils/unique_closable.hpp"
#include "lib/notifs/ToastNotification.hpp"
#include "lib/notifs/win_toasts.hpp"
#include "lib/notifs/NotificationTypes.hpp"
#include "lib/res/strings.h"
#include "lib/ClipboardAction.hpp"

// Application UI :
#include "ClipboardActionView.h"
#include "SearchSuggestionView.h"
#include "SettingsPage.h"

#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.ApplicationModel.DataTransfer.h>
#include <winrt/Windows.ApplicationModel.Resources.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Microsoft.Windows.AppNotifications.h>
#include <winrt/Microsoft.Windows.AppNotifications.Builder.h>
#include <winrt/Windows.System.h>
#include <microsoft.ui.xaml.window.h>
#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/Windows.Globalization.DateTimeFormatting.h>

#include <boost/property_tree/xml_parser.hpp>
#include <boost/regex.hpp>

#include <Shobjidl.h>
#include <ppltasks.h>
#include <pplawait.h>

#include <iostream>
#include <chrono>
#include <sstream>
#include <limits>
#include <format>
#include <map>
#include <ranges>

namespace xaml
{
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Controls;
    using namespace winrt::Microsoft::UI::Windowing;
}

namespace win
{
    using namespace winrt::Microsoft::Windows::AppNotifications::Builder;
    using namespace winrt::Microsoft::Windows::AppNotifications;
    using namespace winrt::Windows::ApplicationModel;
    using namespace winrt::Windows::ApplicationModel::DataTransfer;
    using namespace winrt::Windows::ApplicationModel::Resources;
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;
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
                quickSettingsOpenState,
                searchClosedState,
                searchOpenState
            });

        manager.registerActionCallback([this](std::wstring args)
        {
            LaunchAction(args);
        });

        manager.registerActivatedCallback([this]()
        {
            appWindow.Show();
        });

        clipboardTriggerViews.VectorChanged([this](auto&&, auto&&)
        {
            if (noClipboardTriggersToDisplayState.active() && clipboardTriggerViews.Size() > 0)
            {
                visualStateManager.goToState(displayClipboardTriggersState);
            }
        });
    }

    MainPage::MainPage(const winrt::Microsoft::UI::Windowing::AppWindow& appWindow) : MainPage()
    {
        this->appWindow = appWindow;
    }

    MainPage::~MainPage()
    {
        win::Clipboard::ContentChanged(clipboardContentChangedToken);
    }

    win::Collections::IObservableVector<winrt::ClipboardManager::ClipboardActionView> MainPage::Actions()
    {
        return clipboardActionViews;
    }

    void MainPage::Actions(const win::Collections::IObservableVector<winrt::ClipboardManager::ClipboardActionView>& value)
    {
        clipboardActionViews = value;
    }

    winrt::Windows::Foundation::Collections::IObservableVector<winrt::ClipboardManager::ClipboardActionEditor> MainPage::ClipboardTriggers()
    {
        return clipboardTriggerViews;
    }

    void MainPage::ClipboardTriggers(const winrt::Windows::Foundation::Collections::IObservableVector<winrt::ClipboardManager::ClipboardActionEditor>& value)
    {
        clipboardTriggerViews = value;
    }

    bool MainPage::OverlayEnabled() const
    {
        return overlayEnabled.get();
    }

    void MainPage::OverlayEnabled(const bool& value)
    {
        overlayEnabled.set(value);
    }

    void MainPage::AppClosing()
    {
        if (SearchActionsIgnoreCaseToggleButton())
        {
            localSettings.insert(L"SearchActionsIgnoreCase", SearchActionsIgnoreCaseToggleButton().IsChecked().GetBoolean());
        }

        auto userFilePath = localSettings.get<std::filesystem::path>(L"UserFilePath");
        if (userFilePath.has_value())
        {
            clip::ClipboardTrigger::saveClipboardTriggers(triggers, userFilePath.value());

            if (clipboardActionViews.Size() > 0
                && localSettings.get<bool>(L"SaveMatchingResults").value_or(true)
                && !HistoryToggleButton().IsChecked().GetBoolean())
            {
                boost::property_tree::wptree tree{};
                boost::property_tree::read_xml(userFilePath.value().string(), tree, boost::property_tree::xml_parser::trim_whitespace);

                auto&& history = boost::property_tree::wptree();
                for (int i = clipboardActionViews.Size() - 1; i >= 0; i--)
                {
                    auto&& view = clipboardActionViews.GetAt(i);
                    try
                    {
                        clip::ClipboardAction clipboardAction{ view.Text(), view.Timestamp() };
                        clipboardAction.save(history);
                    }
                    catch (...)
                    {
                        logger.error(L"Error serializing clipboard actions.");
                        history.add(L"item", std::wstring(view.Text()));
                    }
                }

                tree.put_child(L"settings.history", history);

                boost::property_tree::write_xml(userFilePath.value().string(), tree, std::locale(), boost::property_tree::xml_writer_settings<std::wstring>('\t', 1 ));
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

    void MainPage::UpdateUserFile(const hstring& pathString)
    {
        auto path = std::filesystem::path(std::wstring(pathString));
        if (!LoadUserFile(path))
        {
            MessagesBar().AddWarning(L"ErrorMessage_FailedToLoadUserFile", L"User file cannot be loaded, it might contain invalid data.");
        }
        else
        {
            localSettings.insert(L"UserFilePath", path);

            auto triggersNumber = triggers.size();
            auto message = std::vformat(resLoader.getStdResource(L"UserMessage_XNumberOfTriggersLoaded").value_or(L"{} triggers loaded."), 
                                        std::make_wformat_args(triggersNumber));
            MessagesBar().Add(L"", message, xaml::InfoBarSeverity::Success);
        }
    }

    void MainPage::ReceiveWindowMessage(const uint64_t& message, const uint64_t& param)
    {
        switch (message)
        {
            case WM_ACTIVATE:
                if (LOWORD(param) == WA_INACTIVE && overlayEnabled.get())
                {
                    appWindow.Hide();
                }
                break;
            case WM_CLIPBOARDUPDATE:
                ClipboardContent_Changed(nullptr, nullptr);
                break;
        }
    }


    void MainPage::Restore()
    {
        // Load triggers:
        LoadUserFile(std::nullopt);

        if (localSettings.get<bool>(L"InterpretWMClipboardMessages").value_or(false))
        {
            logger.info(L"Interpreting WM clipboard messages instead of using WASDK events.");
        }
        else
        {
            clipboardContentChangedToken = win::Clipboard::ContentChanged({ this, &MainPage::ClipboardContent_Changed });
            /*win::Clipboard::ContentChanged([this](auto&&, auto&&)
            {
                logger.info(L"Clipboard CONTENT changed.");
            });*/
        }

        try
        {
            // Get activation hot key (shortcut that brings the app window to the foreground):
            auto&& map = localSettings.get<std::map<std::wstring, clip::reg_types>>(L"ActivationHotKey");
            if (!map.empty())
            {
                auto key = std::get<std::wstring>(map[L"Key"]);
                auto mod = std::get<uint32_t>(map[L"Mod"]);
                if (key.size() == 1)
                {
                    activationHotKey = clip::HotKey(mod, key[0]);
                }
            }

            activationHotKey.startListening([this]()
            {
                appWindow.Show();
                if (!SetForegroundWindow(GetWindowFromWindowId(appWindow.Id())))
                {
                    std::wstring message = clip::utils::to_wstring(std::system_category().message(GetLastError()));
                    logger.error(std::vformat(L"SetForegroundWindow failed: ", std::make_wformat_args(message)));
                }
            });
        }
        catch (std::invalid_argument)
        {
            logger.error(L"HotKey invalid argument error.");

            uint32_t index{};
            if (TitleBarGrid().Children().IndexOf(OverlayToggleButton(), index))
            {
                TitleBarGrid().Children().RemoveAt(index);
            }
        }
    }

    void MainPage::ReloadActions()
    {
        try
        {
            // Get the text of any actions that have been created then use 
            // AddAction to re-create the actions with the new triggers.
            auto vector = std::vector<clip::ClipboardAction>();
            for (auto&& view : clipboardActionViews)
            {
                vector.push_back({ view.Text(), view.Timestamp() });
            }

            clipboardActionViews.Clear();
            
            for (auto&& action : vector)
            {
                AddAction(action, false);
            }
        }
        catch (hresult_error)
        {
            logger.error(L"Failed to restore clipboard actions history.");
        }
    }

    bool MainPage::LoadUserFile(std::optional<std::filesystem::path>&& userFilePath)
    {
        logger.info(L"*Loading user file*");

        bool triggersLoaded = false;

        if (!userFilePath)
        {
            userFilePath = localSettings.get<std::filesystem::path>(L"UserFilePath");
        }

        if (userFilePath.has_value() && clip::utils::pathExists(userFilePath.value()))
        {
            if ((triggersLoaded = LoadTriggers(userFilePath.value())))
            {
                LoadHistory(userFilePath.value());

                // Enable file watcher:
                try
                {
                    if (localSettings.get<bool>(L"EnableTriggerFileWatching").value_or(false))
                    {
                        watcher.startWatching(userFilePath.value());
                    }
                }
                catch (std::wstring message)
                {
                    logger.error(L"Error enabling file watcher: " + message);
                }
            }
            else
            {
                logger.info(L"Failed to load triggers.");
            }
        }
        else if (userFilePath)
        {
            // The app has saved a file path, but it doesn't exist anymore.
            logger.info(L"User file path doesn't exist: \"" + userFilePath.value().wstring() + L"\"");

            MessagesBar().AddWarning(L"ErrorMessage_UserFileMoved");
        }

        return triggersLoaded;
    }

    bool MainPage::LoadTriggers(std::filesystem::path& path)
    {
        logger.info(L"*Loading triggers*");

        if (!std::filesystem::exists(path))
        {
            logger.info(L"User file path doesn't exist.");

            MessagesBar().AddError(L"ReloadActionsUserFileNotFoundTitle",
                                   L"Failed to reload triggers",
                                   L"ReloadActionsUserFileNotFoundMessage",
                                   L"Triggers user file has either been moved/deleted or application settings have been cleared.");
            return false;
        }

        try
        {
            triggers = clip::ClipboardTrigger::loadClipboardTriggers(path);
            logger.info(std::format(L"{} triggers in file.", triggers.size()));

            if (!triggers.empty())
            {
                clipboardTriggerViews.Clear();

                bool showError = false;
                std::wstring invalidTriggers{};
                
                for (auto&& trigger : triggers)
                {
                    try
                    {
                        auto view = CreateTriggerView(trigger);
                        clipboardTriggerViews.Append(view);

                        trigger.checkFormat();
                    }
                    catch (clip::ClipboardTriggerFormatException formatExcept)
                    {
                        logger.error(std::format(L"Trigger '{}' has an invalid format.", trigger.label()));

                        showError = true;
                        invalidTriggers += L"\n" + trigger.label();
                    }
                }

                if (!showError)
                {
                    visualStateManager.goToState(displayClipboardTriggersState);
                }
                else
                {
                    MessagesBar().Add(resLoader.getOrAlt(L"InvalidTriggerErrorTitle", clip::res::InvalidTriggerErrorTitle),
                                      resLoader.getOrAlt(L"InvalidTriggerErrorMessage", clip::res::InvalidTriggerErrorMessage) + invalidTriggers,
                                      xaml::InfoBarSeverity::Error);
                }
            }
            else
            {
                MessagesBar().AddWarning(L"UserMessage_NoClipboardTriggers", L"No clipboard triggers");
                visualStateManager.goToState(noClipboardTriggersToDisplayState);
            }

            return true;
        }
        catch (std::invalid_argument invalidArgument)
        {
            MessagesBar().AddError(L"ErrorMessage_TriggersInvalidData", L"Triggers file not available or contains invalid data.");
        }
        catch (boost::property_tree::xml_parser_error xmlParserError)
        {
            if (xmlParserError.line() != 0)
            {
                auto line = xmlParserError.line();
                auto xmlParserErrorMessage = std::vformat(resLoader.getStdResource(L"ErrorMessage_XmlParserError").value_or(L"Triggers file has invalid XML markup data."),
                                                          std::make_wformat_args(line));
                MessagesBar().AddError(xmlParserErrorMessage, std::wstring());
            }
        }
        catch (boost::property_tree::ptree_bad_path badPath)
        {
            hstring message = resLoader.getOrAlt(L"ErrorMessage_InvalidTriggersFile", L"Triggers file has invalid XML markup data.");
            hstring content{};

            auto wpath = badPath.path<boost::property_tree::wpath>();
            auto path = clip::utils::to_wstring(wpath.dump());
            if (path == L"settings.triggers")
            {
                content = resLoader.getOrAlt(L"ErrorMessage_MissingTriggersNode",
                                             L"XML declaration is missing '<triggers>' node.\nCheck settings for an example of a valid XML declaration.");
            }
            else
            {
                // I18N: Message not translated.
                content = L"Path not found: '" + hstring(path) + L"'";
            }

            MessagesBar().Add(message, content, xaml::InfoBarSeverity::Error);
        }

        return false;
    }

    async MainPage::LoadHistory(std::filesystem::path userFilePath)
    {
        co_await resume_background();

        // Load history:
        try
        {
            logger.info(L"*Loading actions history*");

            uint32_t count = 0;
            boost::property_tree::wptree tree{};
            boost::property_tree::read_xml(userFilePath.string(), tree);
            for (auto&& historyItem : tree.get_child(L"settings.history"))
            {
                DispatcherQueue().TryEnqueue([this, action = clip::ClipboardAction(historyItem.second)]()
                {
                    AddAction(action, false);
                });
                
                if (count++ > localSettings.get<uint32_t>(L"ActionHistoryMaxCount").value_or(200))
                {
                    DispatcherQueue().TryEnqueue([this]()
                    {
                        MessagesBar().AddWarning(L"UserMessage_TooManyActions", L"Too many actions in history.");
                    });

                    break;
                }
            }
        }
        catch (const boost::property_tree::ptree_bad_path& badPath)
        {
            logger.error(L"Failed to retreive history from user file: " + clip::utils::to_wstring(badPath.what()));
        }
        catch (const boost::property_tree::xml_parser_error& parserError)
        {
            logger.error(L"Failed to retreive history from user file: " + clip::utils::to_wstring(parserError.what()));
        }
    }
    
    void MainPage::AddAction(const clip::ClipboardAction& action, const bool& notify)
    {
        if (localSettings.get<bool>(L"AddDuplicatedActions").value_or(true)
            || clipboardActionViews.Size() == 0
            || action.text() != clipboardActionViews.GetAt(0).Text())
        {
            auto actionView = winrt::make<ClipboardActionView>(action.text().c_str());
            actionView.Timestamp(clock::from_sys(action.creationTime()));

            std::vector<std::pair<std::wstring, std::wstring>> buttons{};
            if (FindActions(actionView, action.text(), notify, buttons))
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

    bool MainPage::FindActions(winrt::ClipboardManager::ClipboardActionView& actionView, const std::wstring& text, 
                               const bool& notify, std::vector<std::pair<std::wstring, std::wstring>>& buttons)
    {
        const auto matchMode = localSettings.get<clip::MatchMode>(L"TriggerMatchMode");

        bool hasMatch = false;
        for (auto&& trigger : triggers)
        {
            if (trigger.enabled() && trigger.match(text, matchMode))
            {
                hasMatch = true;

                actionView.AddAction(trigger.label(), trigger.format(), trigger.regex().str(), true, 
                                     trigger.useRegexMatchResults(), trigger.regex().flags() & boost::regex_constants::icase);

                if (notify)
                {
                    try
                    {
                        const auto url = trigger.formatTrigger(text);
                        buttons.push_back({ trigger.label(), url });
                    }
                    catch (std::invalid_argument formatError)
                    {
                        logger.error(clip::utils::to_wstring(formatError.what()));

                        MessagesBar().AddWarning(trigger.label(), L"Failed to create format for trigger " + trigger.label());
                    }
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
            logger.info(L"Notifications are silenced, not sending one.");
            return;
        }

        auto durationType = localSettings.get<clip::notifs::NotificationDurationType>(L"NotificationDurationType").value_or(clip::notifs::NotificationDurationType::Default);
        auto scenarioType = localSettings.get<clip::notifs::NotificationScenarioType>(L"NotificationScenarioType").value_or(clip::notifs::NotificationScenarioType::Default);
        auto soundType = localSettings.get<clip::notifs::NotificationSoundType>(L"NotificationSoundType").value_or(clip::notifs::NotificationSoundType::Default);

        clip::notifs::ToastNotification notif{};

        if (!notif.tryAddButtons(buttons))
        {
            notif.addText(std::wstring(resLoader.getOrAlt(L"ToastNotification_TooManyButtons",
                                                          L"Too many triggers matched, open the app to activate the trigger of your choice")));

            notif.addButton(std::wstring(resLoader.getOrAlt(L"ToastNotification_OpenApp", L"Open app")), L"");
        }
        else
        {
            notif.addText(std::wstring(resLoader.getOrAlt(L"ToastNotification_ClickAction", 
                                                          L"Click corresponding trigger to activate it")));
        }

        try
        {
            notif.send(durationType, scenarioType, soundType);
            logger.info(L"Notification sent.");
        }
        catch (hresult_access_denied accessDenied)
        {
            MessagesBar().AddWarning(L"ToastNotification_FailedToSendWarning", L"Failed to send toast notification.");
        }
    }

    void MainPage::LaunchAction(const std::wstring& url)
    {
        logger.info(L"Launching url: " + url);
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
                    // I18N:
                    MessagesBar().AddError(L"", L"Failed to launch url: '" + url + L"'.");
                });
            }
        });
    }

    winrt::async MainPage::AddClipboardItem(const Windows::ApplicationModel::DataTransfer::DataPackageView& content, const bool& notify)
    {
        if (content.Contains(win::StandardDataFormats::Text()))
        {
            clip::utils::ClipboardSourceFinder clipboardSourceFinder{};

            auto&& itemText = co_await content.GetTextAsync();
            if (!itemText.empty())
            {
                // Run triggers on the text:
                AddAction(clip::ClipboardAction(std::wstring(itemText)), notify);
            }
        }
    }

    ClipboardManager::ClipboardActionEditor MainPage::CreateTriggerView(clip::ClipboardTrigger& trigger)
    {
        auto triggerViewer = make<ClipboardManager::implementation::ClipboardActionEditor>();
        triggerViewer.ActionFormat(trigger.format());
        triggerViewer.ActionLabel(trigger.label());
        triggerViewer.ActionRegex(trigger.regex().str());
        triggerViewer.ActionEnabled(trigger.enabled());
        triggerViewer.IgnoreCase((trigger.regex().flags() & boost::regex_constants::icase) == boost::regex_constants::icase);
        triggerViewer.UseSearch(trigger.matchMode() == clip::MatchMode::Search);
        triggerViewer.UseRegexMatchResults(trigger.useRegexMatchResults());

        triggerViewer.IsOn({ this, &MainPage::Editor_Toggled });
        triggerViewer.LabelChanged({ this, &MainPage::Editor_LabelChanged });
        triggerViewer.Changed({ this, &MainPage::Editor_Changed });

        triggerViewer.Removed([this](auto&& sender, auto&&)
        {
            auto&& label = sender.ActionLabel();
            for (size_t i = 0; i < triggers.size(); i++)
            {
                if (triggers[i].label() == label)
                {
                    triggers.erase(triggers.begin() + i);
                    break;
                }
            }

            for (uint32_t i = 0; i < clipboardTriggerViews.Size(); i++)
            {
                auto view = clipboardTriggerViews.GetAt(i).try_as<ClipboardManager::ClipboardActionEditor>();
                if (view && view.ActionLabel() == label)
                {
                    clipboardTriggerViews.RemoveAt(i);
                    break;
                }
            }
        });

        return triggerViewer;
    }

    void MainPage::RefreshSearchBoxSuggestions(std::wstring text)
    {
        auto list = single_threaded_vector<IInspectable>();

        if (!text.empty())
        {
            static const std::map<SearchFilter, std::wstring> filtersPrefixes
            {
                { SearchFilter::Triggers, static_cast<std::wstring>(resLoader.getResource(L"SearchFilter_Triggers").value_or(L"t:")) },
                { SearchFilter::Text, static_cast<std::wstring>(resLoader.getResource(L"SearchFilter_Text").value_or(L"x:")) }
            };

            bool hasFilter = false;
            SearchFilter filter = SearchFilter::Actions;
            if (text.starts_with(filtersPrefixes.at(SearchFilter::Triggers)))
            {
                filter = SearchFilter::Triggers;
                hasFilter = true;
                SearchTriggersToggleButton().IsChecked(true);
                SearchTextToggleButton().IsChecked(false);

                text = text.substr(filtersPrefixes.at(SearchFilter::Triggers).size());
            }
            else if (text.starts_with(filtersPrefixes.at(SearchFilter::Text)))
            {
                filter = SearchFilter::Text;
                hasFilter = true;
                SearchTriggersToggleButton().IsChecked(false);
                SearchTextToggleButton().IsChecked(true);

                text = text.substr(filtersPrefixes.at(SearchFilter::Triggers).size());
            }
            else
            {
                SearchTriggersToggleButton().IsChecked(false);
                SearchTextToggleButton().IsChecked(false);
            }

            FillSearchBoxSuggestions(list, filter, text);
        }
        else
        {
            SearchTriggersToggleButton().IsChecked(false);
            SearchTextToggleButton().IsChecked(false);
        }

        SearchActionsResultsTextBlock().Text(to_hstring(list.Size()));
        SearchActionsListView().ItemsSource(list);
    }

    void MainPage::FillSearchBoxSuggestions(const Windows::Foundation::Collections::IVector<IInspectable>& list, const SearchFilter& filter, std::wstring text)
    {
        try
        {
            if (text.empty())
            {
                text = L".*";
            }

            const auto flags = SearchActionsIgnoreCaseToggleButton().IsChecked().GetBoolean() ? boost::regex_constants::icase : boost::regex_constants::normal;
            const auto regex = boost::wregex(text, flags);

            const hstring searchFilter_TriggersDesc = resLoader.getResource(L"SearchFilter_TriggersDesc").value_or(L"t:");
            const hstring searchFilter_TextDesc = resLoader.getResource(L"SearchFilter_TextDesc").value_or(L"x:");

            std::optional<win::DateTime> dateFilter{};
            if (SearchActionsFilterDateToggleButton().IsChecked())
            {
                if (!SearchActionsDatePickerToggleSwitch().IsOn())
                {
                    SearchActionsDatePicker().SelectedDate(winrt::clock::now());
                }

                dateFilter = SearchActionsDatePicker().Date();

                if (SearchActionsTimePickerToggleSwitch().IsOn())
                {
                    auto timePickerSelectedTime = SearchActionsTimePicker().Time();
                    dateFilter = dateFilter.value() + timePickerSelectedTime;
                }
            }

            if ((filter & SearchFilter::Triggers) == SearchFilter::Triggers)
            {
                std::unique_lock lock{ triggersMutex };
                for (auto&& trigger : triggers)
                {
                    if (boost::regex_search(trigger.label(), regex))
                    {
                        list.Append(MakeTriggerSuggestion(trigger.label(), searchFilter_TriggersDesc));
                    }
                }
            }
            else
            {
                for (auto&& actionView : clipboardActionViews)
                {
                    logger.debug(
                        std::format(
                            L"Date filter: {} | trigger creation time < date filter ? {}", 
                            dateFilter.has_value() ? L"yes" : L"no",
                            (!dateFilter.has_value() || actionView.Timestamp() < dateFilter.value()) ? L"yes" : L"no"
                        )
                    );

                    //boost::wsmatch matches{};
                    auto triggersText = actionView.GetTriggersText();

                    if ((filter & SearchFilter::Text) == SearchFilter::Text)
                    {
                        for (auto&& triggerText : triggersText)
                        {
                            if (boost::regex_search(std::wstring(triggerText), regex)
                                && (!dateFilter.has_value() || actionView.Timestamp() < dateFilter.value()))
                            {
                                list.Append(MakeTextSuggestion(actionView, triggerText, searchFilter_TriggersDesc));
                            }
                        }
                    }

                    if ((filter & SearchFilter::Actions) == SearchFilter::Actions
                        && boost::regex_search(std::wstring(actionView.Text()), regex)
                        && (!dateFilter.has_value() || actionView.Timestamp() < dateFilter.value()))
                    {
                        list.Append(MakeActionSuggestion(filter, actionView, triggersText, searchFilter_TextDesc));
                    }
                }
            }
        }
        catch (boost::regex_error&)
        {
            logger.error(L"! Regex error");
        }
    }

    void MainPage::SetDragRectangles()
    {
        if (!overlayEnabled.get())
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
    }

    ClipboardManager::SearchSuggestionView MainPage::MakeActionSuggestion(const SearchFilter& filter, const ClipboardManager::ClipboardActionView& actionView,
                                                                          const winrt::Windows::Foundation::Collections::IVector<hstring>& triggersText, const hstring& searchFilter_TextDesc)
    {
        ClipboardManager::SearchSuggestionView searchSuggestionView = make<ClipboardManager::implementation::SearchSuggestionView>();
        searchSuggestionView.Icon((filter & SearchFilter::Text) == SearchFilter::Text 
                                  ? box_value(searchFilter_TextDesc)
                                  : box_value(L"action"));
        searchSuggestionView.Suggestion(box_value(actionView.Text()));

        searchSuggestionView.RightTapped([this, tag = single_threaded_vector<hstring>({ actionView.Text(), triggersText.GetAt(0) }), searchSuggestionView]
        (auto&& s, auto&& e)
        {
            logger.debug(L"SearchSuggestionView RightTapped.");
            auto actionText = tag.GetAt(0);
            auto triggerLabel = tag.GetAt(1);

            std::unique_lock lock{ triggersMutex };
            for (auto&& trigger : triggers)
            {
                if (triggerLabel == trigger.label())
                {
                    win::DataPackage dataPackage{};
                    dataPackage.SetText(trigger.formatTrigger(actionText.data()));
                    dataPackage.Properties().ApplicationName(APP_NAMEW);
                    win::Clipboard::SetContent(dataPackage);

                    searchSuggestionView.ShowCopiedToClipboard(triggerLabel);
                }
            }
        });

        xaml::DropDownButton dropdownButton{};
        xaml::MenuFlyout flyout{};
        dropdownButton.Flyout(flyout);
        for (auto&& triggerText : triggersText)
        {
            xaml::MenuFlyoutItem button{};
            button.Text(triggerText);
            auto tag = single_threaded_vector<hstring>();
            tag.Append(actionView.Text());
            tag.Append(triggerText);
            button.Tag(tag);
            button.Click([this](auto&& sender, auto&&)
            {
                win::IVector<hstring> tag = sender.as<xaml::FrameworkElement>().Tag().as<win::IVector<hstring>>();
                auto actionText = tag.GetAt(0);
                auto triggerLabel = tag.GetAt(1);

                std::unique_lock lock{ triggersMutex };
                for (auto&& trigger : triggers)
                {
                    if (triggerLabel == trigger.label())
                    {
                        clip::utils::Launcher launcher{};
                        launcher.launch(trigger, static_cast<std::wstring>(actionText));
                    }
                }
            });
            flyout.Items().Append(button);
        }

        const Windows::Globalization::DateTimeFormatting::DateTimeFormatter formatter
        {
            L"{hour.integer(2)}:{minute.integer(2)} {day.integer}/{month.integer}/{year.full}"
        };
        searchSuggestionView.Subtitle(formatter.Format(actionView.Timestamp()));

        auto size = triggersText.Size();
        auto message = std::vformat(resLoader.getStdResource(L"UserMessage_XNumberOfTriggers").value_or(L"{}"), std::make_wformat_args(size));
        dropdownButton.Content(box_value(message));
        searchSuggestionView.RightContent(dropdownButton);

        return searchSuggestionView;
    }

    ClipboardManager::SearchSuggestionView MainPage::MakeTextSuggestion(const ClipboardManager::ClipboardActionView& actionView, const hstring& triggerText, 
                                                                        const hstring& searchFilter_TriggersDesc)
    {
        ClipboardManager::SearchSuggestionView hostControl = make<ClipboardManager::implementation::SearchSuggestionView>();
        hostControl.Icon(box_value(searchFilter_TriggersDesc));
        hostControl.Suggestion(box_value(triggerText));
        hostControl.Subtitle(actionView.Text());
        return hostControl;
    }

    ClipboardManager::SearchSuggestionView MainPage::MakeTriggerSuggestion(const std::wstring& triggerLabel, const hstring& searchFilter_TriggersDesc)
    {
        ClipboardManager::SearchSuggestionView suggestion = make<ClipboardManager::implementation::SearchSuggestionView>();
        suggestion.Icon(box_value(searchFilter_TriggersDesc));
        suggestion.Suggestion(box_value(triggerLabel));

        std::wstring actions{};
        for (auto&& actionView : clipboardActionViews)
        {
            auto triggersText = actionView.GetTriggersText();
            for (auto&& triggerText : triggersText)
            {
                if (static_cast<std::wstring>(triggerText) == triggerLabel)
                {
                    actions += (actions.empty() ? actionView.Text() : L", " + actionView.Text());
                    break;
                }
            }
        }

        if (!actions.empty())
        {
            suggestion.Subtitle(actions);
        }

        return suggestion;
    }

    Windows::Foundation::IInspectable MainPage::asInspectable()
    {
        return *this;
    }

    async MainPage::ClipboardContent_Changed(const win::IInspectable&, const win::IInspectable&)
    {
        using namespace std::literals;
        static std::chrono::system_clock::time_point lastEntry{};
        auto&& now = std::chrono::system_clock::now();
        auto elapsed = (now - lastEntry);
        lastEntry = now;
        if (elapsed < 50ms)
        {
            logger.info(L"Duplicate clipboard event.");
            co_return;
        }

        auto content = win::Clipboard::GetContent();
        if (content.Properties().ApplicationName() == L"ClipboardManager")
        {
            logger.info(L"Clipboard changed, but the available format is not text or the application that changed the clipboard content is me.");
        }
        else
        {
#ifdef _DEBUG
            clip::utils::clipboard_properties_formatter formatter{};
            logger.debug(L"Clipboard HISTORY changed: " + formatter.format(content));
#endif // DEBUG

            co_await AddClipboardItem(content, true);
        }
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
                        clipboardItem.EditAction(pos, action.label(), action.format(), action.regex().str(), action.enabled(), 
                                                 action.useRegexMatchResults(), action.regex().flags() & boost::regex_constants::icase);
                    }
                }

                break;
            }
        }
    }

    void MainPage::FileWatcher_Changed()
    {
        DispatcherQueue().TryEnqueue([&]()
        {
            if (localSettings.get<bool>(L"ReloadWholeUserFileOnFileChange"))
            {
                LoadUserFile(std::nullopt);
            }
            else
            {
                auto path = localSettings.get<std::filesystem::path>(L"UserFilePath").value_or(std::filesystem::path());
                LoadTriggers(path);
                ReloadActions();
            }
        });
    }

    void MainPage::Editor_Changed(const winrt::ClipboardManager::ClipboardActionEditor& sender, const win::IInspectable& inspectable)
    {
        auto actionLabel = std::wstring(sender.ActionLabel());
        auto format = std::wstring(sender.ActionFormat());
        auto newRegex = std::wstring(sender.ActionRegex());

        auto flags = inspectable.as<int32_t>();
        bool ignoreCase = flags & (1 << 2);
        bool useSearch = flags & (1 << 1);
        bool useRegexMatchResults = flags & 1;

        auto regexFlags = ignoreCase ? boost::regex_constants::icase : 0;

        for (auto&& action : triggers)
        {
            if (action.label() == actionLabel)
            {
                action.format(format);
                action.regex(boost::wregex(newRegex, regexFlags));
                action.updateMatchMode(useSearch ? clip::MatchMode::Search : clip::MatchMode::Match);
                action.useRegexMatchResults(useRegexMatchResults);

                // Edit on every action the trigger:
                for (auto&& clipboardItem : clipboardActionViews)
                {
                    uint32_t pos = 0;
                    if (clipboardItem.IndexOf(pos, action.label()))
                    {
                        clipboardItem.EditAction(pos, action.label(), action.format(), action.regex().str(), 
                                                 action.enabled(), action.useRegexMatchResults(), ignoreCase);
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

    void MainPage::OverlayEnabled_Changed(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs args)
    {
        auto&& presenter = appWindow.Presenter().as<xaml::OverlappedPresenter>();
        if (presenter)
        {
            if (overlayEnabled.get())
            {
                presenter.IsMinimizable(!overlayEnabled.get());
                presenter.IsMaximizable(!overlayEnabled.get());
                presenter.IsResizable(localSettings.get<bool>(L"OverlayIsResizable").value_or(true));
                appWindow.IsShownInSwitchers(localSettings.get<bool>(L"OverlayShownInSwitchers").value_or(true) && activationHotKey.registered());
                appWindow.TitleBar().PreferredHeightOption(Microsoft::UI::Windowing::TitleBarHeightOption::Collapsed);

                visualStateManager.goToState(overlayWindowState);
            }
            else
            {
                presenter.IsMinimizable(localSettings.get<bool>(L"AllowWindowMinimize").value_or(true));
                presenter.IsMaximizable(localSettings.get<bool>(L"AllowWindowMaximize").value_or(false));
                presenter.IsResizable(true);
                appWindow.IsShownInSwitchers(true);
                appWindow.TitleBar().PreferredHeightOption(Microsoft::UI::Windowing::TitleBarHeightOption::Standard);

                visualStateManager.goToState(normalWindowState);
            }
            presenter.IsAlwaysOnTop(overlayEnabled.get());
        }
        else
        {
            logger.error(L"Cannot inspect appWindow's presenter as OverlappedPresenter.");
        }

        raisePropertyChanged(args);
    }


    void MainPage::Page_SizeChanged(win::IInspectable const&, xaml::SizeChangedEventArgs const&)
    {
        SetDragRectangles();
    }

    void MainPage::Page_Loading(xaml::FrameworkElement const&, win::IInspectable const&)
    {
        loaded = false;
        Restore();
    }

    void MainPage::Page_Loaded(win::IInspectable const&, xaml::RoutedEventArgs const&)
    {
        //Restore();

        // Minimize app window if the user requested to :
        if (localSettings.get<bool>(L"StartWindowMinimized").value_or(false))
        {
            if (activationHotKey.registered() && localSettings.get<bool>(L"HideAppWindow").value_or(false))
            {
                overlayEnabled.set(true);
                appWindow.Hide();
            }
            else
            {
                appWindow.Presenter().as<xaml::OverlappedPresenter>().Minimize();
            }
        }

        // Adapt grid window button column from the state of the presenter :
        auto&& presenter = appWindow.Presenter().try_as<xaml::OverlappedPresenter>();
        if (presenter)
        {
            WindowButtonsColumn().Width(xaml::GridLengthHelper::FromPixels(presenter.IsMinimizable() ? 135 : 45));
        }

        // Adapt triggers page UI from the size of the window :
        visualStateManager.goToState(appWindow.Size().Width < 930 ? under1kState : over1kState);
        appWindow.Changed([this](auto, xaml::AppWindowChangedEventArgs args)
        {
            if (args.DidSizeChange())
            {
                if (appWindow.Size().Width < 930)
                {
                    visualStateManager.goToState(under1kState);
                }
                else if (appWindow.Size().Width > 930 && !over1kState.active())
                {
                    visualStateManager.goToState(over1kState);
                }
            }
        });

        // Check app version and display a message to the user if it has gone a major update :
        auto&& appVersionSetting = localSettings.get<std::wstring>(L"CurrentAppVersion");
        clip::utils::AppVersion appVersion{};
        if (appVersionSetting.has_value())
        {
            clip::utils::AppVersion storedAppVersion{ appVersionSetting.value() };
            if (appVersion.major() < storedAppVersion.major())
            {
                logger.info(L"Application has been updated, clearing settings and removing startup task.");

                localSettings.clear();
                clip::notifs::toasts::compat::DesktopNotificationManagerCompat::Uninstall();

                visualStateManager.goToState(applicationUpdatedState);
            }
            else if (localSettings.get<bool>(L"FirstStartup").value_or(true))
            {
                localSettings.insert(L"FirstStartup", false);
                visualStateManager.goToState(firstStartupState);
            }
        }
        localSettings.insert(L"CurrentAppVersion", appVersion.versionString());

        // Check if a user file has been saved, if not show a message to the user :
        if (!localSettings.get<std::wstring>(L"UserFilePath"))
        {
            visualStateManager.goToState(noUserFilePathSavedState);
        }

        // Show a message that the logging backend has not been initialized if it hasn't :
        if (!logger.isLogBackendInitialized() && localSettings.get<bool>(L"LoggingEnabled").value_or(false))
        {
            MessagesBar().AddMessage(L"Logging backend is not initialized.");
        }

        loaded = true;
    }

    winrt::async MainPage::LocateUserFileButton_Click(win::IInspectable const&, xaml::RoutedEventArgs const&)
    {
        win::FileOpenPicker picker{};
        picker.as<IInitializeWithWindow>()->Initialize(GetActiveWindow());
        picker.FileTypeFilter().Append(L".xml");

        auto&& storageFile = co_await picker.PickSingleFileAsync();
        if (storageFile)
        {
            auto&& path = std::filesystem::path(storageFile.Path().data());
            localSettings.insert(L"UserFilePath", path);
            if (LoadUserFile(path))
            {
                visualStateManager.goToState(viewActionsState);
            }
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

        auto&& storageFile = co_await(picker.PickSaveFileAsync());
        if (storageFile)
        {
            std::filesystem::path userFilePath{ storageFile.Path().c_str() };
            localSettings.insert(L"UserFilePath", userFilePath);

            visualStateManager.goToState(openSaveFileState);
            visualStateManager.goToState(userFilePathSavedState);
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
            ClipboardTriggersPivotTeachingTip(),
            ClipboardActionsPivotTeachingTip(),
            SearchButtonTeachingTip(),
            OpenQuickSettingsButtonTeachingTip(),
            ReloadActionsButtonTeachingTip(),
            SilenceNotificationsToggleButtonTeachingTip(),
            HistoryToggleButtonTeachingTip(),
            ClearActionsButtonTeachingTip(),
            ImportClipboardButtonTeachingTip(),
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
            teachingTipIndex = 0;

            // Start tour of triggers:
            bool manuallyAddedAction = false;
            if (clipboardActionViews.Size() == 0)
            {
                manuallyAddedAction = true;
                auto actionView = winrt::make<ClipboardActionView>(L"Clipboard content that sequenceSearch a trigger");
                actionView.AddAction(L"Trigger", L"{}", L".*", true, false, false);
                clipboardActionViews.InsertAt(0, actionView);
            }

            co_await clipboardActionViews.GetAt(0).StartTour();

            if (manuallyAddedAction)
            {
                clipboardActionViews.RemoveAt(0);
            }

            Pivot().SelectedItem(ClipboardTriggersPivotItem());
            TeachingTip2_CloseButtonClick(nullptr, nullptr);
        }
    }

    winrt::async MainPage::TeachingTip2_CloseButtonClick(xaml::TeachingTip const& sender, win::IInspectable const&)
    {
        static size_t index = 1;
        static std::vector<xaml::TeachingTip> teachingTips
        {
            ClipboardTriggersListViewTeachingTip(),
            CommandBarImportButtonTeachingTip(),
            CommandBarSaveButtonTeachingTip(),
            CommandBarReloadTriggersButtonTeachingTip(),
            CommandBarAddTriggerButtonTeachingTip(),
            CommandBarTestRegexButtonTeachingTip()
        };

        if (index < teachingTips.size())
        {
            if (index == 5)
            {
                TriggersCommandBar().IsOpen(true);
            }
            else
            {
                TriggersCommandBar().IsOpen(false);
            }

            teachingTips[index - 1].IsOpen(false);
            teachingTips[index++].IsOpen(true);
        }
        else
        {
            teachingTips[index - 1].IsOpen(false);

            bool manuallyAddedAction = false;
            if (clipboardTriggerViews.Size() == 0)
            {
                visualStateManager.goToState(displayClipboardTriggersState);

                auto triggerView = make<ClipboardActionEditor>();
                triggerView.ActionLabel(L"Test trigger");
                triggerView.ActionFormat(L"https://duckduckgo.com/?q={}");
                triggerView.ActionRegex(L".+");
                triggerView.ActionEnabled(true);
                clipboardTriggerViews.InsertAt(0, triggerView);
                manuallyAddedAction = true;
            }

            co_await clipboardTriggerViews.GetAt(0).as<winrt::ClipboardManager::ClipboardActionEditor>().StartTour();

            if (manuallyAddedAction)
            {
                clipboardTriggerViews.RemoveAt(0);
                visualStateManager.goToState(noClipboardTriggersToDisplayState);
            }

            Pivot().SelectedItem(ClipboardActionsPivotItem());
        }
    }

    void MainPage::OpenQuickSettingsButton_Click(win::IInspectable const&, xaml::RoutedEventArgs const&)
    {
        visualStateManager.switchState(quickSettingsClosedState.group());
    }

    void MainPage::ReloadTriggersButton_Click(win::IInspectable const&, xaml::RoutedEventArgs const&)
    {
        auto path = localSettings.get<std::filesystem::path>(L"UserFilePath").value_or(std::filesystem::path());
        if (LoadTriggers(path))
        {
            ReloadActions();
        }
    }

    async MainPage::CommandBarSaveButton_Click(win::IInspectable const&, xaml::RoutedEventArgs const&)
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
            switch (co_await(UserFileSaveContentDialog().ShowAsync()))
            {
                case xaml::ContentDialogResult::Primary:
                    CreateUserFileButton_Click(nullptr, nullptr);
                    break;
                case xaml::ContentDialogResult::Secondary:
                    LocateUserFileButton_Click(nullptr, nullptr);
                    break;
                case xaml::ContentDialogResult::None:
                default:
                    MessagesBar().AddMessage(L"ErrorMessage_CannotSaveTriggersNoUserFile", L"Cannot save triggers, no user file has been specified for this application.");
                    break;
            }
        }
    }

    void MainPage::ClearActionsButton_Click(win::IInspectable const&, xaml::RoutedEventArgs const& e)
    {
        clipboardActionViews.Clear();
    }

    winrt::async MainPage::ImportFromClipboardButton_Click(win::IInspectable const&, xaml::RoutedEventArgs const& e)
    {
        try
        {
            auto&& clipboardHistory = co_await win::Clipboard::GetHistoryItemsAsync();
            for (auto&& item : clipboardHistory.Items())
            {
                auto&& content = item.Content();
                co_await AddClipboardItem(content, false);
            }
        }
        catch (hresult_error error)
        {
            MessagesBar().AddWarning(L"Warning_FailedToLoadClipboardHistory", L"Failed to load clipboard history.");
        }

        // TODO: Notify user that clipboard content has been imported ?
    }

    winrt::async MainPage::AddTriggerButton_Click(win::IInspectable const&, xaml::RoutedEventArgs const&)
    {
        if (co_await(ClipboardTriggerEditControl().Edit()))
        {
            auto label = std::wstring(ClipboardTriggerEditControl().TriggerLabel());
            auto format = std::wstring(ClipboardTriggerEditControl().TriggerFormat());
            auto ignoreCase = ClipboardTriggerEditControl().IgnoreCase();
            auto regex = boost::wregex(std::wstring(ClipboardTriggerEditControl().TriggerRegex()),
                                       (ignoreCase ? boost::regex_constants::icase : 0u));
            auto useSearch = ClipboardTriggerEditControl().UseSearch();
            auto useRegexMatchResults = ClipboardTriggerEditControl().UseRegexMatchResults();

            auto trigger = clip::ClipboardTrigger(label, format, regex, true);
            trigger.updateMatchMode(useSearch ? clip::MatchMode::Search : clip::MatchMode::Match);
            trigger.useRegexMatchResults(useRegexMatchResults);
            triggers.push_back(trigger);

            //logger.info(L"Created new trigger: ")

            clipboardTriggerViews.InsertAt(0, CreateTriggerView(trigger));
        }
    }

    void MainPage::TestRegexButton_Click(win::IInspectable const&, xaml::RoutedEventArgs const&)
    {
        TestRegexContentDialog().Open();
    }

    void MainPage::OverlayToggleButton_Click(win::IInspectable const&, xaml::RoutedEventArgs const&)
    {
        if (localSettings.get<bool>(L"ShowOverlayWarningMessage").value_or(true))
        {
            OverlayToggleButtonTeachingTip().IsOpen(true);
        }
    }

    void MainPage::CommandBarImportButton_Click(win::IInspectable const&, xaml::RoutedEventArgs const&)
    {
        LocateUserFileButton_Click(nullptr, nullptr);
    }

    void MainPage::OverlayPopupShowWarningCheckBox_Click(win::IInspectable const& sender, xaml::RoutedEventArgs const& e)
    {
        localSettings.insert(L"ShowOverlayWarningMessage", sender.as<xaml::Controls::CheckBox>().IsChecked().GetBoolean());
    }

    void MainPage::OpenSearchButton_Click(win::IInspectable const&, xaml::RoutedEventArgs const&)
    {
        visualStateManager.switchState(searchClosedState.group());

        if (visualStateManager.getCurrentState(searchClosedState.group()).name() == searchClosedState.name())
        {
            SearchBoxGrid().Scale({ 0, 0, 0 });
        }
        else
        {
            SearchBoxGrid().Scale({ 1, 1, 1 });
        }

        // Show actions list view if the search list view is currently visible.
        if (showSearchListViewState.active())
        {
            visualStateManager.goToState(showActionsListViewState);
        }
    }

    void MainPage::SearchActionsAutoSuggestBox_TextChanged(win::IInspectable const& sender, xaml::TextChangedEventArgs const& args)
    {
        RefreshSearchBoxSuggestions(std::wstring(SearchActionsAutoSuggestBox().Text()));
    }

    void MainPage::SearchActionsAutoSuggestBox_GotFocus(win::IInspectable const&, xaml::RoutedEventArgs const&)
    {
        RefreshSearchBoxSuggestions(std::wstring(SearchActionsAutoSuggestBox().Text()));
    }

    void MainPage::SearchBoxGrid_Loading(xaml::FrameworkElement const&, win::IInspectable const&)
    {
        SearchActionsIgnoreCaseToggleButton().IsChecked(localSettings.get<bool>(L"SearchActionsIgnoreCase").value_or(true));
    }

    void MainPage::CompactModeToggleButton_Click(win::IInspectable const&, xaml::RoutedEventArgs const&)
    {
        if (CompactModeToggleButton().IsChecked().GetBoolean())
        {
            for (auto&& view : clipboardActionViews)
            {
                xaml::VisualStateManager::GoToState(view, L"CompactVisual", true);
            }
        }
        else
        {
            for (auto&& view : clipboardActionViews)
            {
                xaml::VisualStateManager::GoToState(view, L"NormalVisual", true);
            }
        }
    }

    void MainPage::SearchActionsListView_DoubleTapped(win::IInspectable const&, winrt::Microsoft::UI::Xaml::Input::DoubleTappedRoutedEventArgs const& e)
    {
        logger.debug(L"Search list view double tapped.");
    }

    void MainPage::RootGrid_ActualThemeChanged(xaml::FrameworkElement const& sender, win::IInspectable const& args)
    {
        if (appWindow.TitleBar().IsCustomizationSupported())
        {
            auto&& titleBar = appWindow.TitleBar();
            auto&& resources = winrt::Application::Current().Resources();

            appWindow.TitleBar().ButtonInactiveForegroundColor(
                resources.TryLookup(winrt::box_value(L"AppTitleBarHoverColor")).as<winrt::Windows::UI::Color>());
            appWindow.TitleBar().ButtonHoverBackgroundColor(
                resources.TryLookup(winrt::box_value(L"ButtonHoverBackgroundColor")).as<winrt::Windows::UI::Color>());
            appWindow.TitleBar().ButtonForegroundColor(
                resources.TryLookup(winrt::box_value(L"ButtonForegroundColor")).as<winrt::Windows::UI::Color>());
            appWindow.TitleBar().ButtonHoverForegroundColor(
                resources.TryLookup(winrt::box_value(L"ButtonForegroundColor")).as<winrt::Windows::UI::Color>());
            appWindow.TitleBar().ButtonPressedForegroundColor(
                resources.TryLookup(winrt::box_value(L"ButtonForegroundColor")).as<winrt::Windows::UI::Color>());
        }
    }

    void MainPage::OverlayCloseButton_Click(win::IInspectable const&, xaml::RoutedEventArgs const&)
    {
        if (appWindow)
        {
            appWindow.Hide();
        }
    }

    void MainPage::QuickSettingsSaveButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        AppClosing();
    }

    void MainPage::SettingsFrame_Loaded(win::IInspectable const&, win::IInspectable const& e)
    {
        auto&& page = make<ClipboardManager::implementation::SettingsPage>(*this);
        SettingsFrame().Content(page);
    }
}


