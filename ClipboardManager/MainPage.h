#pragma once
#include "MainPage.g.h"

#include "lib/Settings.hpp"
#include "lib/ClipboardTrigger.hpp"
#include "lib/FileWatcher.hpp"
#include "lib/HotKey.hpp"
#include "lib/notifs/ToastNotificationHandler.hpp"
#include "lib/utils/ResLoader.hpp"
#include "lib/ui/VisualStateManager.hpp"
#include "lib/ui/ListenablePropertyValue.hpp"
#include "lib/ClipboardAction.hpp"

#include "ClipboardActionEditor.h"

#include <vector>
#include <filesystem>
#include <mutex>

namespace winrt::ClipboardManager::implementation
{
    enum SearchFilter
    {
        Actions = 1 << 0,
        Triggers = 1 << 1,
        Text = 1 << 2,
    };

    struct MainPage : MainPageT<MainPage>, clip::ui::PropertyChangedClass
    {
     public:
        MainPage();
        MainPage(const winrt::Microsoft::UI::Windowing::AppWindow& appWindow);
        ~MainPage();

        winrt::Windows::Foundation::Collections::IObservableVector<winrt::ClipboardManager::ClipboardActionView> Actions();
        void Actions(const winrt::Windows::Foundation::Collections::IObservableVector<winrt::ClipboardManager::ClipboardActionView>& value);
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::ClipboardManager::ClipboardActionEditor> ClipboardTriggers();
        void ClipboardTriggers(const winrt::Windows::Foundation::Collections::IObservableVector<winrt::ClipboardManager::ClipboardActionEditor>& value);
        bool OverlayEnabled() const;
        void OverlayEnabled(const bool& value);

        void AppClosing();
        void UpdateTitleBar();
        void UpdateUserFile(const hstring& pathString);
        void ReceiveWindowMessage(const uint64_t& message, const uint64_t& param);

     private:
        clip::utils::Logger logger{ L"MainPage" };
        clip::ui::VisualState<MainPage> openSaveFileState{ L"CreateNewActions", 0, false };
        clip::ui::VisualState<MainPage> viewActionsState{ L"ViewActions", 0, false };
        clip::ui::VisualState<MainPage> displayClipboardTriggersState{ L"DisplayClipboardTriggers", 1, false };
        clip::ui::VisualState<MainPage> noClipboardTriggersToDisplayState{ L"NoClipboardTriggersToDisplay", 1, false };
        clip::ui::VisualState<MainPage> normalStartupState{ L"NormalStartup", 2, true };
        clip::ui::VisualState<MainPage> firstStartupState{ L"FirstStartup", 2, false };
        clip::ui::VisualState<MainPage> applicationUpdatedState{ L"ApplicationUpdated", 2, false };
        clip::ui::VisualState<MainPage> quickSettingsClosedState{ L"QuickSettingsClosed", 3, true };
        clip::ui::VisualState<MainPage> quickSettingsOpenState{ L"QuickSettingsOpen", 3, false };
        clip::ui::VisualState<MainPage> normalWindowState{ L"NormalWindow", 4, true };
        clip::ui::VisualState<MainPage> overlayWindowState{ L"OverlayWindow", 4, false };
        clip::ui::VisualState<MainPage> under1kState{ L"Under1000", 5, true };
        clip::ui::VisualState<MainPage> over1kState{ L"Over1000", 5, false };
        clip::ui::VisualState<MainPage> searchClosedState{ L"SearchClosed", 6, true };
        clip::ui::VisualState<MainPage> searchOpenState{ L"SearchOpened", 6, false };
        clip::ui::VisualState<MainPage> showActionsListViewState{ L"ShowActionsListView", 7, true };
        clip::ui::VisualState<MainPage> showSearchListViewState{ L"ShowSearchListView", 7, false };
        clip::ui::VisualState<MainPage> userFilePathSavedState{ L"UserFilePathSaved", 8, true };
        clip::ui::VisualState<MainPage> noUserFilePathSavedState{ L"NoUserFilePathSaved", 8, false };

        bool loaded = false;
        size_t teachingTipIndex = 0;
        clip::Settings localSettings{};
        clip::FileWatcher watcher{ std::bind(&MainPage::FileWatcher_Changed, this) };
        clip::HotKey activationHotKey{ MOD_CONTROL, L' ' };
        clip::ui::VisualStateManager<MainPage> visualStateManager{ *this };
        clip::notifs::ToastNotificationHandler manager{};
        clip::utils::ResLoader resLoader{};
        std::timed_mutex triggersMutex{};
        std::vector<clip::ClipboardTrigger> triggers{};
        Microsoft::UI::Windowing::AppWindow appWindow{ nullptr };
        Windows::Foundation::Collections::IObservableVector<ClipboardManager::ClipboardActionView> clipboardActionViews = single_threaded_observable_vector<ClipboardManager::ClipboardActionView>();
        Windows::Foundation::Collections::IObservableVector<ClipboardManager::ClipboardActionEditor> clipboardTriggerViews = single_threaded_observable_vector<winrt::ClipboardManager::ClipboardActionEditor>();
        event_token clipboardContentChangedToken{};
        clip::ui::ListenablePropertyValue<bool> overlayEnabled{ std::bind(&MainPage::OverlayEnabled_Changed, this, std::placeholders::_1) };

        void Restore();
        void AddAction(const clip::ClipboardAction& action, const bool& notify);
        bool FindActions(winrt::ClipboardManager::ClipboardActionView& actionView, const std::wstring& text, 
                         const bool& notify, std::vector<std::pair<std::wstring, std::wstring>>& buttons);
        void SendNotification(const std::vector<std::pair<std::wstring, std::wstring>>& buttons);
        void ReloadActions();
        bool LoadTriggers(std::filesystem::path& path);
        void LaunchAction(const std::wstring& url);
        async AddClipboardItem(const Windows::ApplicationModel::DataTransfer::DataPackageView& content, const bool& notify);
        bool LoadUserFile(std::optional<std::filesystem::path>&& path);
        async LoadHistory(std::filesystem::path path);
        ClipboardManager::ClipboardActionEditor CreateTriggerView(clip::ClipboardTrigger& trigger);
        void RefreshSearchBoxSuggestions(std::wstring text);
        void FillSearchBoxSuggestions(const Windows::Foundation::Collections::IVector<IInspectable>& list, const SearchFilter& searchFilter, std::wstring text);
        void SetDragRectangles();
        ClipboardManager::SearchSuggestionView MakeActionSuggestion(const SearchFilter& filter, const ClipboardManager::ClipboardActionView& actionView,
                                                                    const Windows::Foundation::Collections::IVector<hstring>& triggersText, const hstring& searchFilter_TextDesc);
        ClipboardManager::SearchSuggestionView MakeTextSuggestion(const ClipboardManager::ClipboardActionView& actionView, const hstring& triggerText, 
                                                                  const hstring& searchFilter_TriggersDesc);
        ClipboardManager::SearchSuggestionView MakeTriggerSuggestion(const std::wstring& triggerLabel, const hstring& searchFilter_TriggersDesc);

        // Inherited via PropertyChangedClass
        Windows::Foundation::IInspectable asInspectable() override;

        void FileWatcher_Changed();
        async ClipboardContent_Changed(const winrt::Windows::Foundation::IInspectable& sender, const winrt::Windows::Foundation::IInspectable& args);
        void Editor_Toggled(const winrt::ClipboardManager::ClipboardActionEditor& sender, const bool& isOn);
        void Editor_Changed(const winrt::ClipboardManager::ClipboardActionEditor& sender, const Windows::Foundation::IInspectable& oldFormat);
        void Editor_LabelChanged(const winrt::ClipboardManager::ClipboardActionEditor& sender, const winrt::hstring& oldLabel);
        void OverlayEnabled_Changed(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs args);

    public:
        void Page_SizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::SizeChangedEventArgs const& e);
        void Page_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);
        winrt::async LocateUserFileButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::async CreateUserFileButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void Page_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void StartTourButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::async TeachingTip_CloseButtonClick(winrt::Microsoft::UI::Xaml::Controls::TeachingTip const& sender, winrt::Windows::Foundation::IInspectable const& args);
        winrt::async TeachingTip2_CloseButtonClick(winrt::Microsoft::UI::Xaml::Controls::TeachingTip const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void OpenQuickSettingsButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ReloadTriggersButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        async CommandBarSaveButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ClearActionsButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::async ImportFromClipboardButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::async AddTriggerButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void TestRegexButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void OverlayToggleButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void CommandBarImportButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void OverlayPopupShowWarningCheckBox_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void OpenSearchButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void SearchActionsAutoSuggestBox_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::TextChangedEventArgs const& args);
        void SearchActionsAutoSuggestBox_GotFocus(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void SearchBoxGrid_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void CompactModeToggleButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void SearchActionsListView_DoubleTapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::DoubleTappedRoutedEventArgs const& e);
        void RootGrid_ActualThemeChanged(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void OverlayCloseButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void QuickSettingsSaveButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void SettingsFrame_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
};
}

namespace winrt::ClipboardManager::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
