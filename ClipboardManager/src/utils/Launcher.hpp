#pragma once
#include "src/Settings.hpp"
#include "src/utils/Logger.hpp"

namespace clip::utils
{
    class Launcher
    {
    public:
        Launcher() = default;

        winrt::async launch(const std::wstring& uri);

    private:
        const clip::utils::Logger logger{ L"Launcher" };
        clip::Settings settings{};
    };
}
