#pragma once
#include "MainPage.g.h"

#include "src/Settings.hpp"
#include "src/ClipboardTrigger.hpp"
#include "src/HotKey.hpp"
#include "src/ui/VisualStateManager.hpp"
#include "src/notifs/ToastNotificationHandler.hpp"

#include <winrt/Windows.Foundation.Collections.h>

#include <vector>
#include <filesystem>

namespace winrt::ClipboardManager::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
    public:
        MainPage();

        winrt::Windows::Foundation::Collections::IObservableVector<winrt::ClipboardManager::ClipboardActionView> Actions();
        void Actions(const winrt::Windows::Foundation::Collections::IObservableVector<winrt::ClipboardManager::ClipboardActionView>& value);

        void AppClosing();

        winrt::async Page_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);
        winrt::async LocateUserFileButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::async CreateUserFileButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ClipboadTriggersListPivot_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void ClipboardActionsPivotItem_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void Page_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void StartTourButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::async TeachingTip_CloseButtonClick(winrt::Microsoft::UI::Xaml::Controls::TeachingTip const& sender, winrt::Windows::Foundation::IInspectable const& args);
        winrt::async TeachingTip2_CloseButtonClick(winrt::Microsoft::UI::Xaml::Controls::TeachingTip const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void OpenQuickSettingsButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ReloadTriggersButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void CommandBarSaveButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        const clipmgr::utils::Logger logger{ L"MainPage" };
        const clipmgr::ui::VisualState<MainPage> OpenSaveFileState{ L"CreateNewActions", 0, false };
        const clipmgr::ui::VisualState<MainPage> ViewActionsState{ L"ViewActions", 0, false };
        const clipmgr::ui::VisualState<MainPage> DisplayClipboardTriggersState{ L"DisplayClipboardTriggers", 1, false };
        const clipmgr::ui::VisualState<MainPage> NoClipboardTriggersToDisplayState{ L"NoClipboardTriggersToDisplay", 1, false };
        const clipmgr::ui::VisualState<MainPage> FirstStartupState{ L"FirstStartup", 2, false };
        const clipmgr::ui::VisualState<MainPage> NormalStartupState{ L"NormalStartup", 2, true };
        const clipmgr::ui::VisualState<MainPage> QuickSettingsClosedState{ L"QuickSettingsClosed", 3, true };
        const clipmgr::ui::VisualState<MainPage> QuickSettingsOpenState{ L"QuickSettingsOpen", 3, false };

        winrt::Microsoft::UI::Windowing::OverlappedPresenter presenter{ nullptr };
        bool loaded = false;
        clipmgr::ui::VisualStateManager<MainPage> visualStateManager{ *this };
        clipmgr::Settings localSettings{};
        clipmgr::notifs::ToastNotificationHandler& manager = clipmgr::notifs::ToastNotificationHandler::getDefault();
        clipmgr::HotKey activationHotKey{ MOD_ALT, L' ' };
        std::vector<clipmgr::ClipboardTrigger> triggers{};
        winrt::Windows::ApplicationModel::DataTransfer::Clipboard::ContentChanged_revoker clipboardContentChangedrevoker{};
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::ClipboardManager::ClipboardActionView> clipboardActionViews
            = winrt::single_threaded_observable_vector<winrt::ClipboardManager::ClipboardActionView>();
        size_t teachingTipIndex = 0;

        winrt::async ClipboardContent_Changed(const winrt::Windows::Foundation::IInspectable& sender, const winrt::Windows::Foundation::IInspectable& args);
        void Editor_FormatChanged(const winrt::ClipboardManager::ClipboardActionEditor& sender, const winrt::hstring& oldFormat);
        void Editor_LabelChanged(const winrt::ClipboardManager::ClipboardActionEditor& sender, const winrt::hstring& oldLabel);
        void Editor_Toggled(const winrt::ClipboardManager::ClipboardActionEditor& sender, const bool& isOn);

        void Restore();
        void AddAction(const std::wstring& text, const bool& notify);
        bool FindActions(const winrt::ClipboardManager::ClipboardActionView& actionView, std::vector<std::pair<std::wstring, std::wstring>>& buttons, const std::wstring& text);
        void SendNotification(const std::vector<std::pair<std::wstring, std::wstring>>& buttons);
        void ReloadTriggers();
        bool LoadTriggers(std::filesystem::path& path);
        void LaunchAction(const std::wstring& url);
    };
}

namespace winrt::ClipboardManager::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
