#include "pch.h"
#include "HostControl.h"
#if __has_include("HostControl.g.cpp")
#include "HostControl.g.cpp"
#endif

namespace win
{
    using namespace winrt::Windows::Foundation;
}

namespace xaml
{
    using namespace winrt::Microsoft::UI::Xaml;
}

namespace winrt::ClipboardManager::implementation
{
    IInspectable HostControl::Header()
    {
        return _headerContent;
    }

    void HostControl::Header(const IInspectable& value)
    {
        _headerContent = value;
    }

    IInspectable HostControl::HostContent()
    {
        return _hostContent;
    }

    void HostControl::HostContent(const Windows::Foundation::IInspectable& value)
    {
        _hostContent = value;
    }

    hstring ClipboardManager::implementation::HostControl::Title() const
    {
        return _title;
    }

    void HostControl::Title(const hstring& value)
    {
        _title = value;
    }

    hstring HostControl::Subtitle() const
    {
        return _subtitle;
    }

    void HostControl::Subtitle(const hstring& value)
    {
        _subtitle = value;
    }

    uint32_t HostControl::IconHeight() const
    {
        return _iconHeight;
    }

    void HostControl::IconHeight(const uint32_t& value)
    {
        _iconHeight = value;
    }

    uint32_t HostControl::IconWidth() const
    {
        return _iconWidth;
    }

    void HostControl::IconWidth(const uint32_t& value)
    {
        _iconWidth = value;
    }

    hstring HostControl::IconGlyph() const
    {
        return _iconGlyph;
    }

    void HostControl::IconGlyph(const hstring& value)
    {
        _iconGlyph = value;
    }


    void HostControl::UserControl_IsEnabledChanged(IInspectable const&, xaml::DependencyPropertyChangedEventArgs const& e)
    {
        visualStateManager.goToStateEnabled(e.NewValue().as<bool>());
    }

    void HostControl::UserControl_Loaded(IInspectable const&, xaml::RoutedEventArgs const& e)
    {
        visualStateManager.goToStateEnabled(IsEnabled());

        if (!_title.empty())
        {
            if (!_iconGlyph.empty())
            {
                visualStateManager.goToState(titleSubtitleIconState);
            }
            else
            {
                visualStateManager.goToState(titleSubtitleState);
            }
        }
        else
        {
            visualStateManager.goToState(onlyTitleState);
        }
    }

}
