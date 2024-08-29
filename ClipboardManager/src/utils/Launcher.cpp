#include "pch.h"
#include "Launcher.hpp"

#include "src/Process.hpp"

#include <winrt/Windows.System.h>

#include <format>

namespace winrt
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::System;
}

namespace clip::utils
{
    winrt::async Launcher::launch(const std::wstring& uri)
    {
        if (settings.get<bool>(L"UseCustomProcess").value_or(false))
        {
            auto processString = settings.get<std::wstring>(L"CustomProcessString").value_or(L"");
            if (!processString.empty())
            {
                std::wstring command = std::vformat(processString, std::make_wformat_args(uri));
                logger.info(L"Launching custom process with '" + command + L"'.");

                Process process{ command };
                process.invoke();
            }
            else
            {
                logger.debug(L"User has activated custom process, but custom process string is empty.");
            }
        }
        else
        {
            if (!(uri.starts_with(L"https") || uri.starts_with(L"http")))
            {
                co_await winrt::Launcher::LaunchUriAsync(winrt::Uri(DefaultProtocol + uri));
            }
            else
            {
                co_await winrt::Launcher::LaunchUriAsync(winrt::Uri(uri));
            }
        }
    }
}
