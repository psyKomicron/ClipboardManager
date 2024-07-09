#pragma once
#include "src/Settings.hpp"
#include "src/utils/Logger.hpp"

namespace clipmgr::utils
{
    class Launcher
    {
    public:
        Launcher() = default;

        winrt::async launch(const std::wstring& uri);

    private:
        const clipmgr::utils::Logger logger{ L"Launcher" };
        clipmgr::Settings settings{};
    };
}
