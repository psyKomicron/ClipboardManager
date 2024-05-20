#pragma once
#include "MainPage.g.h"

#include "src/Settings.hpp"
#include "src/ClipboardAction.hpp"
#include "src/ui/VisualStateManager.hpp"

#include <winrt/Windows.Foundation.Collections.h>

#include <vector>

namespace winrt::ClipboardManager::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
    public:
        MainPage();

        winrt::Windows::Foundation::Collections::IObservableVector<winrt::ClipboardManager::ClipboardActionView> Actions();
        void Actions(const winrt::Windows::Foundation::Collections::IObservableVector<winrt::ClipboardManager::ClipboardActionView>& value);

        winrt::async Page_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);
        winrt::async LocateUserFileButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::async CreateUserFileButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ViewActionsButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ClipboadActionsListPivot_Loading(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& args);

    private:
        std::vector<clipmgr::ClipboardAction> actions{};
        winrt::Windows::ApplicationModel::DataTransfer::Clipboard::ContentChanged_revoker clipboardContentChangedrevoker{};
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::ClipboardManager::ClipboardActionView> clipboardActionViews
            = winrt::single_threaded_observable_vector<winrt::ClipboardManager::ClipboardActionView>();
        clipmgr::ui::VisualStateManager<MainPage> visualStateManager{ *this };
        clipmgr::utils::Logger logger{ L"MainPage" };
        clipmgr::Settings localSettings{};
        // Visual states.
        const clipmgr::ui::VisualState<MainPage> NormalActionsState{ L"NormalActions", 0, true };
        const clipmgr::ui::VisualState<MainPage> NoClipboardActionsState{ L"NoClipboardActions", 0, false };
        const clipmgr::ui::VisualState<MainPage> OpenSaveFileState{ L"CreateNewActions", 0, false };
        const clipmgr::ui::VisualState<MainPage> ViewActionsState{ L"ViewActions", 0, false };
        const clipmgr::ui::VisualState<MainPage> NoClipboardActionsToDisplayState{ L"NoClipboardActionsToDisplay", 1, 0 };

        winrt::async ClipboardContent_Changed(const winrt::Windows::Foundation::IInspectable& sender, const winrt::Windows::Foundation::IInspectable& args);
        void App_Closing(const winrt::Windows::Foundation::IInspectable&, const winrt::Windows::Foundation::IInspectable&);
    };
}

namespace winrt::ClipboardManager::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
