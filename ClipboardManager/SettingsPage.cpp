#include "pch.h"
#include "SettingsPage.h"
#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif

#include "src/utils/StartupTask.hpp"

namespace impl = winrt::ClipboardManager::implementation;
namespace winrt
{
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Controls;
    using namespace winrt::Windows::Foundation;
}

#define check_loaded(b) if (!b) return;

void impl::SettingsPage::Page_Loading(winrt::FrameworkElement const&, winrt::IInspectable const&)
{
    clipmgr::utils::StartupTask startupTask{};
    AutoStartToggleSwitch().IsOn(startupTask.isTaskRegistered());
}

void impl::SettingsPage::Page_Loaded(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    loaded = true;
}

void impl::SettingsPage::AutoStartToggleSwitch_Toggled(winrt::IInspectable const& s, winrt::RoutedEventArgs const&)
{
    check_loaded(loaded);

    auto sender = s.as<winrt::ToggleSwitch>();
    clipmgr::utils::StartupTask startupTask{};
    if (!startupTask.isTaskRegistered() && sender.IsOn())
    {
        startupTask.set();
    }
    else if (startupTask.isTaskRegistered() && !sender.IsOn())
    {
        startupTask.remove();
    }
}

