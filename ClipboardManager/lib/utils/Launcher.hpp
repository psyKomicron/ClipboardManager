#pragma once
#include "lib/Settings.hpp"
#include "lib/utils/Logger.hpp"
#include "lib/ClipboardTrigger.hpp"

namespace clip::utils
{
    class Launcher
    {
    public:
        Launcher() = default;

        winrt::async launch(const std::wstring& uri);
        winrt::async launch(clip::ClipboardTrigger& trigger, const std::wstring& text);

    private:
        const wchar_t* DefaultProtocol = L"https://";
        clip::utils::Logger logger{ L"Launcher" };
        clip::Settings settings{};
    };
}
