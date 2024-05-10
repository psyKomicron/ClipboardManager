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
