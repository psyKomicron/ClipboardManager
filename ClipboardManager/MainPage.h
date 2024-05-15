#pragma once
#include "MainPage.g.h"

#include "src/ClipboardAction.hpp"

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

        winrt::Windows::Foundation::IAsyncAction Page_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);

    private:
        std::vector<clipmgr::ClipboardAction> actions{};
        winrt::Windows::ApplicationModel::DataTransfer::Clipboard::ContentChanged_revoker clipboardContentChangedrevoker{};
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::ClipboardManager::ClipboardActionView> clipboardActionViews
            = winrt::single_threaded_observable_vector<winrt::ClipboardManager::ClipboardActionView>();

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
