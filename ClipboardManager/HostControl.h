#pragma once
#include "HostControl.g.h"

#include "src/ui/VisualStateManager.hpp"

namespace winrt::ClipboardManager::implementation
{
    struct HostControl : HostControlT<HostControl>
    {
        HostControl() = default;

        winrt::Windows::Foundation::IInspectable Header();
        void Header(const winrt::Windows::Foundation::IInspectable& value);
        winrt::Windows::Foundation::IInspectable HostContent();
        void HostContent(const winrt::Windows::Foundation::IInspectable& value);
        void UserControl_IsEnabledChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::DependencyPropertyChangedEventArgs const& e);
        void UserControl_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        const clipmgr::ui::VisualStateManager<HostControl> visualStateManager{ *this };
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
