#pragma once
#include "src/utils/helpers.hpp"

#include <winrt/Microsoft.UI.Xaml.Data.h>

#include <boost/regex.hpp>

#ifdef _DEBUG
#include <iostream>
#endif

namespace clip::ui
{
    template<typename T>
    concept PropertyChangedRaisable = requires(T t)
    {
        t(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L""));
    };

    template<typename T, PropertyChangedRaisable Ty = std::function<void(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs)>>
    class ListenablePropertyValue
    {
    public:
        ListenablePropertyValue(const Ty& callback) :
            _callback{ callback }
        {
        }

        T get() const
        {
            return value;
        }

        void set(const T& newValue, std::source_location callerLocation = std::source_location::current())
        {
            value = newValue;
            _callback(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(getPropertyName(callerLocation)));
        }

    private:
        T value{};
        Ty _callback;
        std::optional<winrt::hstring> _propName{};

        winrt::hstring getPropertyName(std::source_location callerLocation)
        {
            if (_propName.has_value())
            {
                return _propName.value();
            }
            else
            {
                const boost::regex functionExtractor{ R"(void __cdecl ([A-z]*::)*([A-z]*)\(.*\))" };
                boost::cmatch match{};

                std::ignore = boost::regex_match(callerLocation.function_name(), match, functionExtractor);
                winrt::hstring functionName{ clip::utils::to_wstring(match[2]) };

                _propName = functionName;

                return functionName;
            }
        }
    };

}

namespace clip::ui
{
    class PropertyChangedClass
    {
    public:
        PropertyChangedClass() = default;

        virtual winrt::Windows::Foundation::IInspectable asInspectable() PURE;

        void raisePropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs args)
        {
            e_propertyChanged(asInspectable(), args);
        }

        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& value)
        {
            return e_propertyChanged.add(value);
        }

        void PropertyChanged(winrt::event_token const& token)
        {
            e_propertyChanged.remove(token);
        }

    private:
        winrt::event<winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> e_propertyChanged;
    };
}