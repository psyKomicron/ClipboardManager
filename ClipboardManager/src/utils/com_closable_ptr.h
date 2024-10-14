#pragma once
#include <winrt/Windows.Foundation.h>
#include <wil/com.h>
#include <memory>
#include <functional>

namespace clip::utils
{
    template<typename T>
    concept Closable = requires(T t)
    {
        static_cast<winrt::Windows::Foundation::IClosable>(t);
        t.Close();
    };

    template<Closable T>
    class unique_closable
    {
    public:
        unique_closable(const T& closable) :
            closable{ closable }
        {
        }

        ~unique_closable()
        {
            closable.Close();
        }

    private:
        T closable{};
    };
}
