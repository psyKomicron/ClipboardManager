#include "pch.h"
#include "MainPage.h"
#if __has_include("MainPage.g.cpp")
#include "MainPage.g.cpp"
#endif

#include "src/Settings.hpp"

#include "ClipboardActionView.h"

#include <winrt/Windows.ApplicationModel.DataTransfer.h>
#include <winrt/Windows.Foundation.Collections.h>

using namespace winrt::ClipboardManager::implementation;

winrt::Windows::Foundation::IAsyncAction MainPage::Page_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const&, winrt::Windows::Foundation::IInspectable const&)
{
    clipmgr::Settings settings{};
    settings.open();
    auto actions = settings.getClipboardActions();

    auto&& clipboardHistory = co_await winrt::Windows::ApplicationModel::DataTransfer::Clipboard::GetHistoryItemsAsync();
    for (auto&& item : clipboardHistory.Items())
    {
        auto&& itemText = co_await item.Content().GetTextAsync();
        ClipboardHistoryListView().Items().Append(box_value(itemText));
        
        /*auto view = winrt::make<winrt::ClipboardManager::implementation::ClipboardActionView>(itemText);
        for (auto&& action : actions)
        {
            view.AddAction(action.format(), action.label(), action.regex().str());
        }*/
    }

    auto view = winrt::make<winrt::ClipboardManager::implementation::ClipboardActionView>(L"This is a test");
    ListView().Items().Append(view);
    
    view = winrt::make<winrt::ClipboardManager::implementation::ClipboardActionView>(L"This is another test");
    ListView().Items().Append(view);
}


