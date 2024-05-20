#pragma once
#include "App.xaml.g.h"

#include <winrt/Microsoft.UI.Xaml.Hosting.h>

namespace winrt::ClipboardManager::implementation
{
    struct App : AppT<App>
    {
        App():
            m_windowsXamlManager(winrt::Microsoft::UI::Xaml::Hosting::WindowsXamlManager::InitializeForCurrentThread())
        {}

    private:
        winrt::Microsoft::UI::Xaml::Hosting::WindowsXamlManager m_windowsXamlManager{ nullptr };
    };
}