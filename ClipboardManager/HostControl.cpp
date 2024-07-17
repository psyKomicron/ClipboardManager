#include "pch.h"
#include "HostControl.h"
#if __has_include("HostControl.g.cpp")
#include "HostControl.g.cpp"
#endif

namespace impl = winrt::ClipboardManager::implementation;
namespace winrt
{
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Windows::Foundation;
}

winrt::IInspectable impl::HostControl::Header()
{
    return _headerContent;
}

void impl::HostControl::Header(const winrt::IInspectable& value)
{
    _headerContent = value;
}

winrt::IInspectable impl::HostControl::HostContent()
{
    return _hostContent;
}

void impl::HostControl::HostContent(const winrt::Windows::Foundation::IInspectable& value)
{
    _hostContent = value;
}


void impl::HostControl::UserControl_IsEnabledChanged(winrt::IInspectable const&, winrt::DependencyPropertyChangedEventArgs const& e)
{
    visualStateManager.goToEnabledState(e.NewValue().as<bool>());
}

void impl::HostControl::UserControl_Loaded(winrt::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    visualStateManager.goToEnabledState(IsEnabled());
}
