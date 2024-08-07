#include "pch.h"
#include "ClipboardActionView.h"
#if __has_include("ClipboardActionView.g.cpp")
#include "ClipboardActionView.g.cpp"
#endif

#include "src/Settings.hpp"
#include "src/utils/Launcher.hpp"

#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.ApplicationModel.DataTransfer.h>
#include <winrt/Microsoft.UI.Input.h>

#include <ppltasks.h>
#include <pplawait.h>

#include <vector>
#include <format>

namespace implementation = winrt::ClipboardManager::implementation;
namespace ui
{
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Controls;
}
namespace win
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::Windows::System;
    using namespace winrt::Windows::ApplicationModel::DataTransfer;
}

implementation::ClipboardActionView::ClipboardActionView()
{
    visualStateManager.initializeStates(
        {
            OptionsClosedState,
            OptionsOpenState
        });
}

implementation::ClipboardActionView::ClipboardActionView(const winrt::hstring& text) : ClipboardActionView()
{
    _text = text;
}

winrt::hstring implementation::ClipboardActionView::Text() const
{
    return _text;
}

void implementation::ClipboardActionView::Text(const winrt::hstring& value)
{
    _text = value;
}

winrt::event_token implementation::ClipboardActionView::Removed(const event_removed_t& handler)
{
    return e_removed.add(handler);
}

void implementation::ClipboardActionView::Removed(const winrt::event_token& token)
{
    e_removed.remove(token);
}

void implementation::ClipboardActionView::AddAction(const winrt::hstring& format, const winrt::hstring& label, const winrt::hstring& regex, const bool& enabled)
{
    actions.push_back(clip::ClipboardTrigger(std::wstring(label), std::wstring(format), boost::wregex(std::wstring(regex)), enabled));
}

void implementation::ClipboardActionView::AddActions(const winrt::Windows::Foundation::IInspectable& inspectable)
{
    AddActions(inspectable.as<win::IVector<win::IVector<winrt::hstring>>>());
}

void implementation::ClipboardActionView::AddActions(const actions_t actions)
{
    for (auto&& action : actions)
    {
        auto label = action.GetAt(0);
        auto format = action.GetAt(1);
        auto regex = action.GetAt(2);

        this->actions.push_back(
            clip::ClipboardTrigger(std::wstring(label), std::wstring(format), boost::wregex(std::wstring(regex)), true)
        );
    }
}

void implementation::ClipboardActionView::EditAction(const uint32_t& pos, const winrt::hstring& _format, const winrt::hstring& _label, const winrt::hstring& _regex, const bool& enabled)
{
    auto format = std::wstring(_format);
    auto label = std::wstring(_label);
    auto regex = std::wstring(_regex);

    auto& action = actions[pos];
    auto oldLabel = action.label();

    action.format(format);
    action.label(label);
    action.regex(boost::wregex(regex));

    // Reload triggers:
    for (uint32_t i = 0; i < ActionsGridView().Items().Size(); i++)
    {
        auto&& item = ActionsGridView().Items().GetAt(i);
        if (auto content = item.try_as<hstring>())
        {
            ActionsGridView().Items().SetAt(i, box_value(_label));
            break;
        }
    }
}

bool implementation::ClipboardActionView::IndexOf(uint32_t& pos, const winrt::hstring& format, const winrt::hstring& label, const winrt::hstring& regex, const bool& enabled)
{
    clip::ClipboardTrigger action{ std::wstring(label), std::wstring(format), boost::wregex(std::wstring(regex)), enabled };
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

winrt::async implementation::ClipboardActionView::StartTour()
{
    visualStateManager.goToState(OptionsOpenState);

    RootGridTeachingTip().IsOpen(true);
 
    auto ptr = &teachingTipsWaitFlag;
    auto func = [ptr]() -> void
    {
        ptr->wait(false);
        ptr->clear();
    };

    co_await concurrency::create_task(std::move(func));
}


void implementation::ClipboardActionView::UserControl_Loading(ui::FrameworkElement const&, win::IInspectable const&)
{
    // Create buttons for triggers:
    for (auto&& action : actions)
    {
        ActionsGridView().Items().InsertAt(0, box_value(action.label()));
    }
}

void implementation::ClipboardActionView::OpenOptionsButton_Click(win::IInspectable const&, ui::RoutedEventArgs const&)
{
    visualStateManager.switchState(0, true);
}


void implementation::ClipboardActionView::HyperlinkButton_Click(win::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    auto&& label = sender.as<ui::Control>().Tag().try_as<hstring>();
    if (label.has_value() && !label.value().empty())
    {
        for (auto&& action : actions)
        {
            if (label.value() == action.label())
            {
                clip::utils::Launcher launcher{};
                auto text = std::wstring(_text);
                launcher.launch(std::vformat(action.format(), std::make_wformat_args(text)));
            }
        }
    }
}

void implementation::ClipboardActionView::FormatLinkButton_Click(win::IInspectable const&, ui::RoutedEventArgs const&)
{
    if (actions.size() == 1)
    {
        win::DataPackage dataPackage{};
        auto text = std::wstring(_text);
        dataPackage.SetText(std::vformat(actions[0].format(), std::make_wformat_args(text)));
        dataPackage.Properties().ApplicationName(L"ClipboardManager");
        win::Clipboard::SetContent(dataPackage);
    }
}

void implementation::ClipboardActionView::RemoveActionButton_Click(win::IInspectable const&, ui::RoutedEventArgs const&)
{
    e_removed(*this, nullptr);
}

void implementation::ClipboardActionView::TeachingTip_ButtonClick(ui::Controls::TeachingTip const& sender, win::IInspectable const& args)
{
    static size_t teachingTipIndex = 1;
    static std::vector<ui::TeachingTip> teachingTips
    {
        RootGridTeachingTip(),
        LabelTextBlockTeachingTip(),
        ActionsGridViewTeachingTip(),
        OpenOptionsButtonTeachingTip(),
        FormatLinkButtonTeachingTip(),
        RemoveActionButtonTeachingTip()
    };

    if (teachingTipIndex < teachingTips.size())
    {
        teachingTips[teachingTipIndex - 1].IsOpen(false);
        teachingTips[teachingTipIndex++].IsOpen(true);
    }
    else
    {
        teachingTips[teachingTipIndex - 1].IsOpen(false);

        teachingTipsWaitFlag.test_and_set();
        teachingTipsWaitFlag.notify_all();
    }
}

void implementation::ClipboardActionView::RootGrid_PointerEntered(win::IInspectable const&, ui::Input::PointerRoutedEventArgs const&)
{
    visualStateManager.goToState(PointerOverState);
    ProtectedCursor(winrt::Microsoft::UI::Input::InputSystemCursor::Create(winrt::Microsoft::UI::Input::InputSystemCursorShape::Hand));
}

void implementation::ClipboardActionView::RootGrid_PointerExited(win::IInspectable const&, ui::Input::PointerRoutedEventArgs const&)
{
    visualStateManager.goToState(NormalState);
    ProtectedCursor(winrt::Microsoft::UI::Input::InputSystemCursor::Create(winrt::Microsoft::UI::Input::InputSystemCursorShape::Arrow));
}

void implementation::ClipboardActionView::RootGrid_PointerPressed(win::IInspectable const&, ui::Input::PointerRoutedEventArgs const& e)
{
    if (e.GetCurrentPoint(*this).Properties().IsLeftButtonPressed())
    {
        visualStateManager.goToState(PointerPressedState);

        clip::utils::Launcher launcher{};
        if (actions.size() > 0)
        {
            auto&& action = actions[0];
            auto text = std::wstring(_text);
            launcher.launch(std::vformat(action.format(), std::make_wformat_args(text)));
        }
    }

}

void implementation::ClipboardActionView::RootGrid_PointerReleased(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e)
{
    // TODO: Implement or remove.
}
