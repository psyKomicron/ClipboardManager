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
        using event_removed_t = Windows::Foundation::TypedEventHandler<ClipboardManager::ClipboardActionView, Windows::Foundation::IInspectable>;
        using actions_t = Windows::Foundation::Collections::IVector<Windows::Foundation::Collections::IVector<hstring>>;

        ClipboardActionView();
        ClipboardActionView(const hstring& text);

        hstring Text() const;
        void Text(const hstring& value);
        Windows::Foundation::DateTime Timestamp();
        void Timestamp(const Windows::Foundation::DateTime& value);

        event_token Removed(const event_removed_t& handler);
        void Removed(const event_token& token);

        void AddAction(const hstring& label, const hstring& format,
                       const hstring& regex, const bool& enabled, const bool& useRegexMatchResults,
                       const bool& ignoreCase);
        void EditAction(const uint32_t& pos, const hstring& label, const hstring& format, 
                        const hstring& regex, const bool& enabled, const bool& useRegexMatchResults,
                        const bool& ignoreCase);
        bool IndexOf(uint32_t& pos, const hstring& label);
        Windows::Foundation::Collections::IVector<hstring> GetTriggersText();

        async StartTour();

    private:
        static clip::utils::ResLoader resLoader;

        clip::ui::VisualStateManager<ClipboardActionView> visualStateManager{ *this };
        using VisualState = clip::ui::VisualState<ClipboardActionView>;
        VisualState optionsClosedState{ L"OptionsClosed", 0, true };
        VisualState optionsOpenState{ L"OptionsOpen", 0, false };
        VisualState normalState{ L"Normal", 1, true };
        VisualState pointerOverState{ L"PointerOver", 1, false };
        VisualState pointerPressedState{ L"PointerPressed", 1, false };
        VisualState normalVisualState{ L"NormalVisual", 2, true };
        VisualState compactVisualState{ L"CompactVisual", 2, true };
        std::atomic_flag teachingTipsWaitFlag{};
        clip::utils::Logger logger{ L"ClipboardActionView" };
        hstring _text{};
        std::vector<clip::ClipboardTrigger> triggers{};
        Windows::Foundation::DateTime _timestamp{};

        event<event_removed_t> e_removed{};

        void AddTriggerButton(clip::ClipboardTrigger& trigger);
        void CopyLinkToClipboard();

    public:
        void UserControl_Loading(Microsoft::UI::Xaml::FrameworkElement const& sender, Windows::Foundation::IInspectable const& args);
        void OpenOptionsButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void HyperlinkButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void FormatLinkButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RemoveActionButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void TeachingTip_ButtonClick(Microsoft::UI::Xaml::Controls::TeachingTip const& sender, Windows::Foundation::IInspectable const& args);
        void RootGrid_PointerEntered(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void RootGrid_PointerExited(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void RootGrid_PointerPressed(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void ContentGrid_PointerReleased(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
    };
}

namespace winrt::ClipboardManager::factory_implementation
{
    struct ClipboardActionView : ClipboardActionViewT<ClipboardActionView, implementation::ClipboardActionView>
    {
    };
}
