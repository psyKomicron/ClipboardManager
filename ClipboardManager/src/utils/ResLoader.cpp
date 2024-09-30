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
        catch (winrt::hresult_error error)
        {
            std::wcout << L"Failed to create instance of winrt::Windows::ApplicationModel::Resources::ResourceLoader: " << error.message().data() << std::endl;
            OutputDebugStringW(error.message().c_str());
        }
    }

    std::optional<winrt::hstring> ResLoader::getNamedResource(const winrt::hstring& name)
    {
        if (resourceLoader != nullptr)
        {
            try
            {
                return resourceLoader.GetString(name);
            }
            catch (winrt::hresult_error)
            {
                std::wcerr << L"'getNamedResource' Failed to instanciate or get string from resources." << std::endl;
            }
        }

        return {};
    }

    winrt::hstring ResLoader::getOrAlt(const winrt::hstring& name, const winrt::hstring& alt)
    {
        return getNamedResource(name).value_or(alt);
    }
}