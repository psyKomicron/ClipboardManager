#include "pch.h"
#include "ClipboardHistoryItemView.h"
#if __has_include("ClipboardHistoryItemView.g.cpp")
#include "ClipboardHistoryItemView.g.cpp"
#endif


namespace winrt::ClipboardManager::implementation
{
    void ClipboardHistoryItemView::HostContent(const Windows::Foundation::IInspectable& value)
    {
        _hostContent = value;
    }

    Windows::Foundation::IInspectable ClipboardHistoryItemView::HostContent() const
    {
        return _hostContent;
    }
}
