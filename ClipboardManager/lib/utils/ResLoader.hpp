#pragma once
#include "lib/utils/Logger.hpp"

#include <winrt/Windows.ApplicationModel.Resources.h>

#include <optional>

namespace clip::utils
{
    class ResLoader
    {
    public:
        ResLoader();

        std::optional<winrt::hstring> getResource(const winrt::hstring& name);
        std::optional<std::wstring> getStdResource(const std::wstring_view& name);
        winrt::hstring getOrAlt(const winrt::hstring& name, const winrt::hstring& alt);

    private:
        Logger logger{ L"ResLoader" };
        winrt::Windows::ApplicationModel::Resources::ResourceLoader resourceLoader{ nullptr };
    };
}
