#pragma once
#include "MainPage.g.h"

#include "src/Settings.hpp"
#include "src/ClipboardTrigger.hpp"
#include "src/HotKey.hpp"
#include "src/notifs/ToastNotificationHandler.hpp"
#include "src/utils/ResLoader.hpp"
#include "src/ui/VisualStateManager.hpp"
#include "src/FileWatcher.hpp"

#include <winrt/Windows.Media.Ocr.h>
#include <winrt/Windows.Storage.Streams.h>

#include <vector>
#include <filesystem>
#include <mutex>

namespace winrt::ClipboardManager::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
     public:
        MainPage();
        MainPage(const winrt::Microsoft::UI::Windowing::AppWindow& appWindow);

        winrt::Windows::Foundation::Collections::IObservableVector<winrt::ClipboardManager::ClipboardActionView> Actions();
        void Actions(const winrt::Windows::Foundation::Collections::IObservableVector<winrt::ClipboardManager::ClipboardActionView>& value);
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::ClipboardManager::ClipboardActionEditor> ClipboardTriggers();
        void ClipboardTriggers(const winrt::Windows::Foundation::Collections::IObservableVector<winrt::ClipboardManager::ClipboardActionEditor>& value);

        void AppClosing();
        void UpdateTitleBar();

        void Page_SizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::SizeChangedEventArgs const& e);
        winrt::async Page_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);
        winrt::async LocateUserFileButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::async CreateUserFileButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ClipboadTriggersListPivot_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& args);
        winrt::async Page_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::async ClipboardHistoryListView_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void StartTourButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::async TeachingTip_CloseButtonClick(winrt::Microsoft::UI::Xaml::Controls::TeachingTip const& sender, winrt::Windows::Foundation::IInspectable const& args);
        winrt::async TeachingTip2_CloseButtonClick(winrt::Microsoft::UI::Xaml::Controls::TeachingTip const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void OpenQuickSettingsButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ReloadTriggersButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void CommandBarSaveButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ClearActionsButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::async ImportFromClipboardButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::async AddTriggerButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void TestRegexButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void OverlayToggleButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void CommandBarImportButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

     private:
        const clip::utils::Logger logger{ L"MainPage" };
        const clip::ui::VisualState<MainPage> openSaveFileState{ L"CreateNewActions", 0, false };
        const clip::ui::VisualState<MainPage> viewActionsState{ L"ViewActions", 0, false };
        const clip::ui::VisualState<MainPage> displayClipboardTriggersState{ L"DisplayClipboardTriggers", 1, false };
        const clip::ui::VisualState<MainPage> noClipboardTriggersToDisplayState{ L"NoClipboardTriggersToDisplay", 1, false };
        const clip::ui::VisualState<MainPage> normalStartupState{ L"NormalStartup", 2, true };
        const clip::ui::VisualState<MainPage> firstStartupState{ L"FirstStartup", 2, false };
        const clip::ui::VisualState<MainPage> applicationUpdatedState{ L"ApplicationUpdated", 2, false };
        const clip::ui::VisualState<MainPage> quickSettingsClosedState{ L"QuickSettingsClosed", 3, true };
        const clip::ui::VisualState<MainPage> quickSettingsOpenState{ L"QuickSettingsOpen", 3, false };
        const clip::ui::VisualState<MainPage> normalWindowState{ L"NormalWindow", 4, true };
        const clip::ui::VisualState<MainPage> overlayWindowState{ L"OverlayWindow", 4, false };
        const clip::ui::VisualState<MainPage> under1kState{ L"Under1000", 5, true };
        const clip::ui::VisualState<MainPage> over1kState{ L"Over1000", 5, false };

        bool loaded = false;
        bool updated = false;
        size_t teachingTipIndex = 0;
        clip::Settings localSettings{};
        clip::FileWatcher watcher{ std::bind(&MainPage::FileWatcher_Changed, this) };
        clip::HotKey activationHotKey{ MOD_ALT, L' ' };
        clip::ui::VisualStateManager<MainPage> visualStateManager{ *this };
        clip::notifs::ToastNotificationHandler manager{};
        clip::utils::ResLoader resLoader{};
        std::vector<clip::ClipboardTrigger> triggers{};
        winrt::Microsoft::UI::Windowing::AppWindow appWindow{ nullptr };
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::ClipboardManager::ClipboardActionView> clipboardActionViews = winrt::single_threaded_observable_vector<winrt::ClipboardManager::ClipboardActionView>();
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::ClipboardManager::ClipboardActionEditor> clipboardTriggerViews = winrt::single_threaded_observable_vector<winrt::ClipboardManager::ClipboardActionEditor>();
        winrt::event_token clipboardContentChangedToken{};

        winrt::async ClipboardContent_Changed(const winrt::Windows::Foundation::IInspectable& sender, const winrt::Windows::Foundation::IInspectable& args);
        void Editor_Toggled(const winrt::ClipboardManager::ClipboardActionEditor& sender, const bool& isOn);
        void Editor_Changed(const winrt::ClipboardManager::ClipboardActionEditor& sender, const Windows::Foundation::IInspectable& oldFormat);
        void Editor_LabelChanged(const winrt::ClipboardManager::ClipboardActionEditor& sender, const winrt::hstring& oldLabel);
        void FileWatcher_Changed();

        void Restore();
        void AddAction(const std::wstring& text, const bool& notify);
        bool FindActions(const winrt::ClipboardManager::ClipboardActionView& actionView, std::vector<std::pair<std::wstring, std::wstring>>& buttons, const std::wstring& text);
        void SendNotification(const std::vector<std::pair<std::wstring, std::wstring>>& buttons);
        void ReloadTriggers();
        bool LoadTriggers(std::filesystem::path& path);
        void LaunchAction(const std::wstring& url);
        winrt::async LoadClipboardHistory();
        winrt::async AddClipboardItem(const Windows::ApplicationModel::DataTransfer::DataPackageView& content, const bool& runTriggers);
        bool LoadUserFile(const std::filesystem::path& path);
        void CreateTriggerViews();
    };
}

namespace winrt::ClipboardManager::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
