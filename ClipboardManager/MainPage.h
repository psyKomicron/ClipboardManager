#pragma once
#include "MainPage.g.h"

#include "src/ClipboardAction.hpp"

#include <vector>

namespace winrt::ClipboardManager::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
        MainPage() = default;

        winrt::Windows::Foundation::IAsyncAction Page_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);

    private:
        std::vector<clipmgr::ClipboardAction> actions{};

        winrt::async ClipboardContent_Changed(const winrt::Windows::Foundation::IInspectable& sender, const winrt::Windows::Foundation::IInspectable& args);
    };
}

namespace winrt::ClipboardManager::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
