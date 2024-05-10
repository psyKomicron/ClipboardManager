#pragma once
#include "HostControl.g.h"

namespace winrt::ClipboardManager::implementation
{
    struct HostControl : HostControlT<HostControl>
    {
        HostControl() = default;

        winrt::Windows::Foundation::IInspectable Header();
        void Header(const winrt::Windows::Foundation::IInspectable& value);
        winrt::Windows::Foundation::IInspectable HostContent();
        void HostContent(const winrt::Windows::Foundation::IInspectable& value);

    private:
        winrt::Windows::Foundation::IInspectable _headerContent{};
        winrt::Windows::Foundation::IInspectable _hostContent{};
    };
}

namespace winrt::ClipboardManager::factory_implementation
{
    struct HostControl : HostControlT<HostControl, implementation::HostControl>
    {
    };
}
