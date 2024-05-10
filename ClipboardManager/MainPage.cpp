#include "pch.h"
#include "MainPage.h"
#if __has_include("MainPage.g.cpp")
#include "MainPage.g.cpp"
#endif

#include "src/Settings.hpp"

#include "ClipboardActionView.h"

#include <winrt/Windows.ApplicationModel.DataTransfer.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.System.h>

#include <iostream>

namespace impl = winrt::ClipboardManager::implementation;

namespace winrt
{
    using namespace winrt::Windows::ApplicationModel::DataTransfer;
    using namespace winrt::Windows::System;
    using namespace winrt::Windows::Foundation;
}

winrt::Windows::Foundation::IAsyncAction impl::MainPage::Page_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const&, winrt::Windows::Foundation::IInspectable const&)
{
    //TODO: Implement settings "page".
    try
    {
        clipmgr::Settings settings{};
        settings.open();
        actions = settings.getClipboardActions();
    }
    catch (std::runtime_error)
    {
    }

    auto&& clipboardHistory = co_await winrt::Clipboard::GetHistoryItemsAsync();
    for (auto&& item : clipboardHistory.Items())
    {
        auto&& itemText = co_await item.Content().GetTextAsync();
        ClipboardHistoryListView().Items().Append(box_value(itemText));
    }

    winrt::Clipboard::ContentChanged({ this, &MainPage::ClipboardContent_Changed });
}

winrt::async impl::MainPage::ClipboardContent_Changed(const winrt::Windows::Foundation::IInspectable& sender, const winrt::Windows::Foundation::IInspectable& args)
{
    OutputDebugStringW(L"[MainPage]  Clipboard content changed.\n");

    auto clipboardContent = winrt::Clipboard::GetContent();
    if (clipboardContent.Contains(winrt::StandardDataFormats::Text()))
    {
        auto&& text = co_await clipboardContent.GetTextAsync();
        auto actionView = winrt::make<::impl::ClipboardActionView>(text);

        bool hasMatch = false;
        for (auto&& action : actions)
        {
            if (action.match(std::wstring(text)))
            {
                actionView.AddAction(action.format(), action.label(), L"");
                hasMatch = true;
            }
        }

        if (hasMatch)
        {
            ListView().Items().InsertAt(0, actionView);
        }
    }
}


