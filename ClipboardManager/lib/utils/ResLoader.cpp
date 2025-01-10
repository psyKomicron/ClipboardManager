#include "pch.h"
#include "ResLoader.hpp"

#include <iostream>

namespace clip::utils
{
    ResLoader::ResLoader()
    {
        try
        {
            resourceLoader = winrt::Windows::ApplicationModel::Resources::ResourceLoader();
        }
        catch (winrt::hresult_error err)
        {
            if (err.code() == 0x80070002)
            {
                logger.error(L"Failed to create ResourceLoader, 'resources.pri' doesn't exist.");
            }
        }
    }

    std::optional<winrt::hstring> ResLoader::getResource(const winrt::hstring& name)
    {
        if (resourceLoader != nullptr)
        {
            try
            {
                auto res = resourceLoader.GetString(name);
                if (!res.empty())
                {
                    return res;
                }
            }
            catch (winrt::hresult_error)
            {
                logger.error(L"'getResource' Failed to instanciate or get string from resources.");
            }
        }

        return std::nullopt;
    }

    std::optional<std::wstring> ResLoader::getStdResource(const std::wstring_view& name)
    {
        if (resourceLoader != nullptr)
        {
            try
            {
                std::wstring res = static_cast<std::wstring>(resourceLoader.GetString(name));
                if (!res.empty())
                {
                    return res;
                }
            }
            catch (winrt::hresult_error)
            {
                logger.error(L"'getResource' Failed to instanciate or get string from resources.");
            }
        }

        return std::nullopt;
    }

    winrt::hstring ResLoader::getOrAlt(const winrt::hstring& name, const winrt::hstring& alt)
    {
        return getResource(name).value_or(alt);
    }
}