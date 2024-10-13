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

    std::optional<winrt::hstring> ResLoader::getNamedResource(const winrt::hstring& name)
    {
        if (resourceLoader.has_value())
        {
            try
            {
                return resourceLoader.value().GetString(name);
            }
            catch (winrt::hresult_error)
            {
                logger.error(L"'getNamedResource' Failed to instanciate or get string from resources.");
            }
        }
        return {};
    }

    winrt::hstring ResLoader::getOrAlt(const winrt::hstring& name, const winrt::hstring& alt)
    {
        return getNamedResource(name).value_or(alt);
    }
}