#pragma once

#include "ClipboardHistoryItemView.g.h"

namespace winrt::ClipboardManager::implementation
{
    struct ClipboardHistoryItemView : ClipboardHistoryItemViewT<ClipboardHistoryItemView>
    {
    public:
        ClipboardHistoryItemView() = default;

        void HostContent(const Windows::Foundation::IInspectable& value);
        Windows::Foundation::IInspectable HostContent() const;

    private:
        Windows::Foundation::IInspectable _hostContent{};
    };
}

namespace winrt::ClipboardManager::factory_implementation
{
    struct ClipboardHistoryItemView : ClipboardHistoryItemViewT<ClipboardHistoryItemView, implementation::ClipboardHistoryItemView>
    {
    };
}
