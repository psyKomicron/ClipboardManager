#include "pch.h"
#include "MainPage.h"
#if __has_include("MainPage.g.cpp")
#include "MainPage.g.cpp"
#endif

#include <winrt/Windows.ApplicationModel.DataTransfer.h>
#include <winrt/Windows.Foundation.Collections.h>

using namespace winrt::ClipboardManager::implementation;

winrt::Windows::Foundation::IAsyncAction MainPage::Page_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args)
{
    auto&& clipboardHistory = co_await winrt::Windows::ApplicationModel::DataTransfer::Clipboard::GetHistoryItemsAsync();
    for (auto&& item : clipboardHistory.Items())
    {
        auto&& itemText = co_await item.Content().GetTextAsync();
        DispatcherQueue().TryEnqueue([this, itemText]()
        {
            ListView().Items().Append(winrt::box_value(itemText));
        });
    }
}


