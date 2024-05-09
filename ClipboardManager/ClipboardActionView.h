#pragma once
#include "ClipboardActionView.g.h"

#include "src/ClipboardAction.hpp"

namespace winrt::ClipboardManager::implementation
{
    struct ClipboardActionView : ClipboardActionViewT<ClipboardActionView>
    {
    public:
        ClipboardActionView() = default;
        ClipboardActionView(const winrt::hstring& text);

        winrt::hstring Text() const;
        void Text(const winrt::hstring& value);

        void AddAction(const winrt::hstring& format, const winrt::hstring& label, const winrt::hstring& regex);

        void UserControl_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);

    private:
        std::vector<clipmgr::ClipboardAction> actions{};
        winrt::hstring _text{};
    };
}

namespace winrt::ClipboardManager::factory_implementation
{
    struct ClipboardActionView : ClipboardActionViewT<ClipboardActionView, implementation::ClipboardActionView>
    {
    };
}
