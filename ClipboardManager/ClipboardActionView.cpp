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
#include <format>

namespace impl = winrt::ClipboardManager::implementation;

namespace winrt
{
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Controls;
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;
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

void impl::ClipboardActionView::AddAction(const winrt::hstring& format, const winrt::hstring& label, const winrt::hstring& regex, const bool& enabled)
{
    actions.push_back(clipmgr::ClipboardAction(std::wstring(label), std::wstring(format), boost::wregex(std::wstring(regex)), enabled));
}

void winrt::ClipboardManager::implementation::ClipboardActionView::AddActions(const winrt::Windows::Foundation::IInspectable& inspectable)
{
    AddActions(inspectable.as<winrt::IVector<winrt::IVector<winrt::hstring>>>());
}

void impl::ClipboardActionView::AddActions(const actions_t actions)
{
    for (auto&& action : actions)
    {
        auto label = action.GetAt(0);
        auto format = action.GetAt(1);
        auto regex = action.GetAt(2);

        this->actions.push_back(
            clipmgr::ClipboardAction(std::wstring(label), std::wstring(format), boost::wregex(std::wstring(regex)), true)
        );
    }
}

impl::actions_t impl::ClipboardActionView::GetActions()
{
    actions_t loadedActions = single_threaded_vector<winrt::IVector<winrt::hstring>>();
    for (auto&& action : actions)
    {
        auto loadedAction = single_threaded_vector<winrt::hstring>();
        loadedAction.Append(action.label());
        loadedAction.Append(action.format());
        loadedAction.Append(action.regex().str());
        loadedActions.Append(loadedAction);
    }

    return loadedActions;
}

void impl::ClipboardActionView::EditAction(const uint32_t& pos, const winrt::hstring& _format, const winrt::hstring& _label, const winrt::hstring& _regex, const bool& enabled)
{
    auto format = std::wstring(_format);
    auto label = std::wstring(_label);
    auto regex = std::wstring(_regex);
    auto& action = actions[pos];
    action.format(format);
    action.label(label);
    action.regex(boost::wregex(regex));

    ActionsGridView().Items().Clear();
    UserControl_Loading(nullptr, nullptr);
}

bool impl::ClipboardActionView::IndexOf(uint32_t& pos, const winrt::hstring& format, const winrt::hstring& label, const winrt::hstring& regex, const bool& enabled)
{
    clipmgr::ClipboardAction action{ std::wstring(label), std::wstring(label), boost::wregex(std::wstring(regex)), enabled };
    for (size_t i = 0; i < actions.size(); i++)
    {
        if (actions[i] == action)
        {
            pos = i;
            return true;
        }
    }
    return false;
}


void impl::ClipboardActionView::UserControl_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const&, winrt::Windows::Foundation::IInspectable const&)
{
    // Create buttons for actions:
    for (auto&& action : actions)
    {
        ActionsGridView().Items().InsertAt(0, box_value(action.label()));
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
                auto text = std::wstring(_text);
                auto wstring = std::vformat(action.format(), std::make_wformat_args(text));
                winrt::Launcher::LaunchUriAsync(winrt::Uri(wstring));
            }
        }
    }
}

void impl::ClipboardActionView::FormatLinkButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    if (actions.size() == 1)
    {
        winrt::DataPackage dataPackage{};
        auto text = std::wstring(_text);
        dataPackage.SetText(std::vformat(actions[0].format(), std::make_wformat_args(text)));
        winrt::Clipboard::SetContent(dataPackage);
    }
}

void impl::ClipboardActionView::RemoveActionButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    e_removed(*this, nullptr);
}
