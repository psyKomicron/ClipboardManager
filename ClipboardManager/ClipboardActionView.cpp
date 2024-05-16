#include "pch.h"
#include "ClipboardActionView.h"
#if __has_include("ClipboardActionView.g.cpp")
#include "ClipboardActionView.g.cpp"
#endif

#include "src/Settings.hpp"

#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.ApplicationModel.DataTransfer.h>

#include <vector>

namespace impl = winrt::ClipboardManager::implementation;

namespace winrt
{
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Controls;
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::System;
    using namespace winrt::Windows::ApplicationModel::DataTransfer;
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

winrt::event_token impl::ClipboardActionView::Removed(const event_removed_t& handler)
{
    return e_removed.add(handler);
}

void impl::ClipboardActionView::Removed(const winrt::event_token& token)
{
    e_removed.remove(token);
}

void impl::ClipboardActionView::AddAction(const winrt::hstring& format, const winrt::hstring& label, const winrt::hstring& regex)
{
    actions.push_back(clipmgr::ClipboardAction(std::wstring(label), std::wstring(format), std::wstring(regex)));
}


void impl::ClipboardActionView::UserControl_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const&, winrt::Windows::Foundation::IInspectable const&)
{
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

void impl::ClipboardActionView::FormatLinkButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    /*winrt::DataPackage dataPackage{};
    dataPackage.SetText(std::vformat(action.format(), std::make_wformat_args(std::wstring(_text))));
    winrt::Clipboard::SetContent()*/
}

void impl::ClipboardActionView::RemoveActionButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    e_removed(*this, nullptr);
}
