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
    public:
        using event_removed_t = winrt::Windows::Foundation::TypedEventHandler<winrt::ClipboardManager::ClipboardActionView, winrt::Windows::Foundation::IInspectable>;
        using actions_t = winrt::Windows::Foundation::Collections::IVector<winrt::Windows::Foundation::Collections::IVector<winrt::hstring>>;

        ClipboardActionView();
        ClipboardActionView(const winrt::hstring& text);

        winrt::hstring Text() const;
        void Text(const winrt::hstring& value);

        winrt::event_token Removed(const event_removed_t& handler);
        void Removed(const winrt::event_token& token);

        void AddAction(const winrt::hstring& label, const winrt::hstring& format,
                       const winrt::hstring& regex, const bool& enabled, const bool& useRegexMatchResults,
                       const bool& ignoreCase);
        void EditAction(const uint32_t& pos, const winrt::hstring& label, const winrt::hstring& format, 
                        const winrt::hstring& regex, const bool& enabled, const bool& useRegexMatchResults,
                        const bool& ignoreCase);
        bool IndexOf(uint32_t& pos, const winrt::hstring& label);
        winrt::Windows::Foundation::Collections::IVector<hstring> GetTriggersText();

        winrt::async StartTour();

    private:
        using VisualState = clip::ui::VisualState<ClipboardActionView>;
        VisualState optionsClosedState{ L"OptionsClosed", 0, true };
        VisualState optionsOpenState{ L"OptionsOpen", 0, false };
        VisualState normalState{ L"Normal", 1, true };
        VisualState pointerOverState{ L"PointerOver", 1, false };
        VisualState pointerPressedState{ L"PointerPressed", 1, false };
        VisualState normalVisualState{ L"NormalVisual", 2, true };
        VisualState compactVisualState{ L"CompactVisual", 2, true };

        static clip::utils::ResLoader resLoader;

        std::atomic_flag teachingTipsWaitFlag{};
        clip::utils::Logger logger{ L"ClipboardActionView" };
        winrt::hstring _text{};
        std::vector<clip::ClipboardTrigger> triggers{};
        clip::ui::VisualStateManager<ClipboardActionView> visualStateManager{ *this };

        winrt::event<event_removed_t> e_removed{};

        void AddTriggerButton(clip::ClipboardTrigger& trigger);
        void CopyLinkToClipboard();

    public:
        void UserControl_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void OpenOptionsButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void HyperlinkButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void FormatLinkButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RemoveActionButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void TeachingTip_ButtonClick(winrt::Microsoft::UI::Xaml::Controls::TeachingTip const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void RootGrid_PointerEntered(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void RootGrid_PointerExited(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void RootGrid_PointerPressed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void ContentGrid_PointerReleased(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
    };
}

namespace winrt::ClipboardManager::factory_implementation
{
    struct ClipboardActionView : ClipboardActionViewT<ClipboardActionView, implementation::ClipboardActionView>
    {
    };
}
