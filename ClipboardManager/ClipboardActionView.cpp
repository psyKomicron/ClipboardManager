#include "pch.h"
#include "ClipboardActionView.h"
#if __has_include("ClipboardActionView.g.cpp")
#include "ClipboardActionView.g.cpp"
#endif

#include "src/Settings.hpp"

#include <winrt/Windows.Storage.h>

using namespace winrt::ClipboardManager::implementation;

ClipboardActionView::ClipboardActionView(const winrt::hstring& text)
{
    _text = text;
}

winrt::hstring ClipboardActionView::Text() const
{
    return _text;
}

void ClipboardActionView::Text(const winrt::hstring& value)
{
    _text = value;
}

void ClipboardActionView::AddAction(const winrt::hstring& format, const winrt::hstring& label, const winrt::hstring& regex)
{
    actions.push_back(clipmgr::ClipboardAction(std::wstring(label), std::wstring(format), std::wstring(regex)));
}


void ClipboardActionView::UserControl_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const&, winrt::Windows::Foundation::IInspectable const&)
{
    clipmgr::Settings settings{};
    settings.open();
}
