#pragma once
#include "ClipboardActionView.g.h"

#include "src/ClipboardTrigger.hpp"
#include "src/ui/VisualStateManager.hpp"

#include <atomic>

namespace winrt::ClipboardManager::implementation
{
    struct ClipboardActionView : ClipboardActionViewT<ClipboardActionView>
    {
        using event_removed_t = winrt::Windows::Foundation::TypedEventHandler<winrt::ClipboardManager::ClipboardActionView, winrt::Windows::Foundation::IInspectable>;
        using actions_t = winrt::Windows::Foundation::Collections::IVector<winrt::Windows::Foundation::Collections::IVector<winrt::hstring>>;

    public:
        ClipboardActionView();
        ClipboardActionView(const winrt::hstring& text);

        winrt::hstring Text() const;
        void Text(const winrt::hstring& value);

        winrt::event_token Removed(const event_removed_t& handler);
        void Removed(const winrt::event_token& token);

        void AddAction(const winrt::hstring& format, const winrt::hstring& label, const winrt::hstring& regex, const bool& enabled);
        void AddActions(const winrt::Windows::Foundation::IInspectable& inspectable);
        void AddActions(const actions_t actions);
        void EditAction(const uint32_t& pos, const winrt::hstring& format, const winrt::hstring& label, const winrt::hstring& regex, const bool& enabled);
        bool IndexOf(uint32_t& pos, const winrt::hstring& format, const winrt::hstring& label, const winrt::hstring& regex, const bool& enabled);

        winrt::async StartTour();

        void UserControl_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void OpenOptionsButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void HyperlinkButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void FormatLinkButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RemoveActionButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void TeachingTip_ButtonClick(winrt::Microsoft::UI::Xaml::Controls::TeachingTip const& sender, winrt::Windows::Foundation::IInspectable const& args);

    private:
        const clipmgr::ui::VisualState<ClipboardActionView> OptionsClosedState{ L"OptionsClosed", 0, true };
        const clipmgr::ui::VisualState<ClipboardActionView> OptionsOpenState{ L"OptionsOpen", 0, false };
        std::vector<clipmgr::ClipboardTrigger> actions{};
        winrt::hstring _text{};
        clipmgr::ui::VisualStateManager<ClipboardActionView> visualStateManager{ *this };
        winrt::event<event_removed_t> e_removed{};
        std::atomic_flag teachingTipsWaitFlag{};
    };
}

namespace winrt::ClipboardManager::factory_implementation
{
    struct ClipboardActionView : ClipboardActionViewT<ClipboardActionView, implementation::ClipboardActionView>
    {
    };
}
