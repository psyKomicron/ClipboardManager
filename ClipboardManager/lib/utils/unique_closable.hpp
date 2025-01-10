#pragma once
#include <winrt/Windows.Foundation.h>
#include <wil/com.h>
#include <memory>
#include <functional>

namespace clip::utils
{
    template<typename T>
    concept is_closable = requires(T t)
    {
        t.Close();
    };

    template<is_closable T>
    class unique_closable
    {
    public:
        unique_closable(T* closable) :
            closable{ closable }
        {
        }

        ~unique_closable()
        {
            closable->Close();
        }

    private:
        T* closable{};
    };
}
