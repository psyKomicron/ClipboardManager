#pragma once
#include "ClipboardActionView.g.h"

#include "src/ClipboardTrigger.hpp"
#include "src/ui/VisualStateManager.hpp"
#include "src/utils/Logger.hpp"
#include "src/utils/ResLoader.hpp"

#include <atomic>

namespace winrt::ClipboardManager::implementation
{
    struct ClipboardActionView : ClipboardActionViewT<ClipboardActionView>
    {
    private:
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
        void EditAction(const uint32_t& pos, const winrt::hstring& label, const winrt::hstring& format, const winrt::hstring& regex, const bool& enabled);
        bool IndexOf(uint32_t& pos, const winrt::hstring& label);

        winrt::async StartTour();

        void UserControl_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void OpenOptionsButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void HyperlinkButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void FormatLinkButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RemoveActionButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void TeachingTip_ButtonClick(winrt::Microsoft::UI::Xaml::Controls::TeachingTip const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void RootGrid_PointerEntered(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void RootGrid_PointerExited(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void RootGrid_PointerPressed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void RootGrid_PointerReleased(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void EditActionButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        using VisualState = clip::ui::VisualState<ClipboardActionView>;
        const VisualState OptionsClosedState{ L"OptionsClosed", 0, true };
        const VisualState OptionsOpenState{ L"OptionsOpen", 0, false };
        const VisualState NormalState{ L"Normal", 1, true };
        const VisualState PointerOverState{ L"PointerOver", 1, false };
        const VisualState PointerPressedState{ L"PointerPressed", 1, false };

        static clip::utils::ResLoader resLoader;

        std::atomic_flag teachingTipsWaitFlag{};
        clip::utils::Logger logger{ L"ClipboardActionView" };
        winrt::hstring _text{};
        std::vector<clip::ClipboardTrigger> actions{};
        clip::ui::VisualStateManager<ClipboardActionView> visualStateManager{ *this };

        winrt::event<event_removed_t> e_removed{};

        void AddTriggerButton(clip::ClipboardTrigger& trigger);
    };
}

namespace winrt::ClipboardManager::factory_implementation
{
    struct ClipboardActionView : ClipboardActionViewT<ClipboardActionView, implementation::ClipboardActionView>
    {
    };
}
