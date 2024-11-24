#pragma once
#include "HostControl.g.h"

#include "src/ui/VisualStateManager.hpp"
//#include "src/ui/ListenablePropertyValue.hpp"

namespace winrt::ClipboardManager::implementation
{
    struct HostControl : HostControlT<HostControl>
    {
    public:
        HostControl() = default;

        winrt::Windows::Foundation::IInspectable Header();
        void Header(const winrt::Windows::Foundation::IInspectable& value);
        winrt::Windows::Foundation::IInspectable HostContent();
        void HostContent(const winrt::Windows::Foundation::IInspectable& value);
        hstring Title() const;
        void Title(const hstring& value);
        hstring Subtitle() const;
        void Subtitle(const hstring& value);
        uint32_t IconHeight() const;
        void IconHeight(const uint32_t& value);
        uint32_t IconWidth() const;
        void IconWidth(const uint32_t& value);
        hstring IconGlyph() const;
        void IconGlyph(const hstring& value);

    private:
        const clip::ui::VisualState<HostControl> onlyTitleState{ L"OnlyHeaderState", 1, true };
        const clip::ui::VisualState<HostControl> titleSubtitleState{ L"TitleSubtitleState", 1, false };
        const clip::ui::VisualState<HostControl> titleSubtitleIconState{ L"TitleSubtitleIconState", 1, false };
        clip::ui::VisualStateManager<HostControl> visualStateManager{ *this };
        Windows::Foundation::IInspectable _headerContent{};
        Windows::Foundation::IInspectable _hostContent{};
        hstring _title{};
        hstring _subtitle{};
        uint32_t _iconHeight = 20;
        uint32_t _iconWidth = 20;
        hstring _iconGlyph{};

    public:
        void UserControl_IsEnabledChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::DependencyPropertyChangedEventArgs const& e);
        void UserControl_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::ClipboardManager::factory_implementation
{
    struct HostControl : HostControlT<HostControl, implementation::HostControl>
    {
    };
}
