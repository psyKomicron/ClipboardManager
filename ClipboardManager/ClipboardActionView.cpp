#include "pch.h"
#include "ClipboardActionView.h"
#if __has_include("ClipboardActionView.g.cpp")
#include "ClipboardActionView.g.cpp"
#endif

#include "src/Settings.hpp"

#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.System.h>

#include <vector>

namespace impl = winrt::ClipboardManager::implementation;

namespace winrt
{
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Controls;
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::System;
}

impl::ClipboardActionView::ClipboardActionView()
{
    visualStateManager.initializeStates(
        {
            OptionsClosedState,
            OptionsOpenState
        });
}

impl::ClipboardActionView::ClipboardActionView(const winrt::hstring& text) : ClipboardActionView()
{
    _text = text;
}

winrt::hstring impl::ClipboardActionView::Text() const
{
    return _text;
}

void impl::ClipboardActionView::Text(const winrt::hstring& value)
{
    _text = value;
}

void impl::ClipboardActionView::AddAction(const winrt::hstring& format, const winrt::hstring& label, const winrt::hstring& regex)
{
    actions.push_back(clipmgr::ClipboardAction(std::wstring(label), std::wstring(format), std::wstring(regex)));
}


void impl::ClipboardActionView::UserControl_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const&, winrt::Windows::Foundation::IInspectable const&)
{
    clipmgr::Settings settings{};
    settings.open();

    // Create buttons for actions:
    for (auto&& action : actions)
    {
        ActionsGridView().Items().InsertAt(0, box_value(action.label()));
        //ActionsGridView().Items().Append(box_value(action.label()));
    }
}

void impl::ClipboardActionView::OpenOptionsButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    visualStateManager.switchState(0, true);
}


void impl::ClipboardActionView::HyperlinkButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    auto&& label = sender.as<winrt::Control>().Tag().try_as<hstring>();
    if (label.has_value() && !label.value().empty())
    {
        for (auto&& action : actions)
        {
            if (label.value() == action.label())
            {
                auto wstring = std::vformat(action.format(), std::make_wformat_args(std::wstring(_text)));
                winrt::Launcher::LaunchUriAsync(winrt::Uri(wstring));
            }
        }
    }
}
