#pragma once
#include "src/utils/Logger.hpp"

#include <winrt/Windows.ApplicationModel.Resources.h>

#include <optional>

namespace clip::utils
{
    class ResLoader
    {
    public:
        ResLoader();

        std::optional<winrt::hstring> getNamedResource(const winrt::hstring& name);
        winrt::hstring getOrAlt(const winrt::hstring& name, const winrt::hstring& alt);

    private:
        Logger logger{ L"ResLoader" };
        std::optional<winrt::Windows::ApplicationModel::Resources::ResourceLoader> resourceLoader{};
    };
}
