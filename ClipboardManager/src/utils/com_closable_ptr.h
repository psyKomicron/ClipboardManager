#pragma once
#include <winrt/Windows.Foundation.h>
#include <wil/com.h>
#include <memory>
#include <functional>

namespace clip::utils
{
    /*inline void __stdcall closeIClosable(winrt::Windows::Foundation::IClosable* source)
    {
        source->Close();
    }*/
    
    inline void __stdcall closeIClosable(winrt::Windows::Foundation::IClosable source)
    {
        source.Close();
    }

    using unique_closable = std::unique_ptr<winrt::Windows::Foundation::IClosable, std::function<void(winrt::Windows::Foundation::IClosable)>>;
    
    /*using unique_closable_call = 
        wil::unique_com_call<
        winrt::Windows::Foundation::IClosable, 
        decltype(&winrt::Windows::Foundation::IClosable::Close), 
        &winrt::Windows::Foundation::IClosable::Close>;*/
}
