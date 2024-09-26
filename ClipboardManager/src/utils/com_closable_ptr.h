#pragma once
#include <winrt/Windows.Foundation.h>
#include <wil/com.h>
#include <memory>
#include <functional>

namespace clip::utils
{
    inline void __stdcall closeIClosable(winrt::Windows::Foundation::IClosable* source)
    {
        source->Close();
    }

    using unique_closable = std::unique_ptr<winrt::Windows::Foundation::IClosable, std::function<void(winrt::Windows::Foundation::IClosable*)>>;
    using shared_closable = std::shared_ptr<winrt::Windows::Foundation::IClosable>;
    using iclosable_delete = decltype(closeIClosable);
}
