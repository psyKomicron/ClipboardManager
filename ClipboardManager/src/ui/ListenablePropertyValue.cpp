#include "pch.h"
#include "ListenablePropertyValue.hpp"

winrt::event_token clip::ui::PropertyChangedClass::PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& value)
{
    return e_propertyChanged.add(value);
}

void clip::ui::PropertyChangedClass::PropertyChanged(winrt::event_token const& token)
{
    e_propertyChanged.remove(token);
}

void clip::ui::PropertyChangedClass::raisePropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs args)
{
    e_propertyChanged(asInspectable(), args);
}
