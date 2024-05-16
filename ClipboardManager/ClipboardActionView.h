#pragma once
#include "ClipboardActionView.g.h"

#include "src/ClipboardAction.hpp"
#include "src/ui/VisualStateManager.hpp"

namespace winrt::ClipboardManager::implementation
{
    using event_removed_t = winrt::Windows::Foundation::TypedEventHandler<winrt::ClipboardManager::ClipboardActionView, winrt::Windows::Foundation::IInspectable>;

    struct ClipboardActionView : ClipboardActionViewT<ClipboardActionView>
    {
    public:
        ClipboardActionView();
        ClipboardActionView(const winrt::hstring& text);

        winrt::hstring Text() const;
        void Text(const winrt::hstring& value);

        winrt::event_token Removed(const event_removed_t& handler);
        void Removed(const winrt::event_token& token);

        void AddAction(const winrt::hstring& format, const winrt::hstring& label, const winrt::hstring& regex);

        void UserControl_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void OpenOptionsButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void HyperlinkButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void FormatLinkButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RemoveActionButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        const clipmgr::ui::VisualState<ClipboardActionView> OptionsClosedState{ L"OptionsClosed", 0, true };
        const clipmgr::ui::VisualState<ClipboardActionView> OptionsOpenState{ L"OptionsOpen", 0, false };
        std::vector<clipmgr::ClipboardAction> actions{};
        winrt::hstring _text{};
        clipmgr::ui::VisualStateManager<ClipboardActionView> visualStateManager{ *this };
        winrt::event<event_removed_t> e_removed{};
    };
}

namespace winrt::ClipboardManager::factory_implementation
{
    struct ClipboardActionView : ClipboardActionViewT<ClipboardActionView, implementation::ClipboardActionView>
    {
    };
}
