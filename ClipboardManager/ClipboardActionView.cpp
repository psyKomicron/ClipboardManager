#include "pch.h"
#include "ClipboardActionView.h"
#if __has_include("ClipboardActionView.g.cpp")
#include "ClipboardActionView.g.cpp"
#endif

#include "src/Settings.hpp"
#include "src/utils/Launcher.hpp"
#include "src/res/strings.h"

#include <winrt/Windows.Storage.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.ApplicationModel.DataTransfer.h>
#include <winrt/Microsoft.UI.Input.h>

#include <ppltasks.h>
#include <pplawait.h>

#include <vector>
#include <format>

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

namespace winrt::ClipboardManager::implementation
{
    ClipboardActionView::ClipboardActionView()
    {
        visualStateManager.initializeStates(
            {
                OptionsClosedState,
                OptionsOpenState
            });
    }

    ClipboardActionView::ClipboardActionView(const winrt::hstring& text) : ClipboardActionView()
    {
        _text = text;
    }

    winrt::hstring ClipboardActionView::Text() const
    {
        return _text;
    }

    void ClipboardActionView::Text(const winrt::hstring& value)
    {
        _text = value;
    }

    winrt::event_token ClipboardActionView::Removed(const event_removed_t& handler)
    {
        return e_removed.add(handler);
    }

    void ClipboardActionView::Removed(const winrt::event_token& token)
    {
        e_removed.remove(token);
    }

    void ClipboardActionView::AddAction(const winrt::hstring& format, const winrt::hstring& label, const winrt::hstring& regex, const bool& enabled)
    {
        auto trigger = clip::ClipboardTrigger(std::wstring(label), std::wstring(format), boost::wregex(std::wstring(regex)), enabled);
        try
        {
            trigger.checkFormat();
            actions.push_back(std::move(trigger));
        }
        catch (clip::ClipboardTriggerFormatException formatException)
        {
            /*hstring message{};
            switch (formatException.code())
            {
                case clip::FormatExceptionCode::MissingOpenBraces:
                    message = resLoader.getOrAlt(L"StringFormatError_MissingBraces", clip::res::StringFormatError_MissingBraces);
                    break;
                case clip::FormatExceptionCode::MissingClosingBraces:
                    message = resLoader.getOrAlt(L"StringFormatError_MissingOpener", clip::res::StringFormatError_MissingOpener);
                    break;
                case clip::FormatExceptionCode::InvalidFormat:
                case clip::FormatExceptionCode::Unknown:
                default:
                    message = resLoader.getOrAlt(L"StringFormatError_InvalidFormat", L"Invalid format string");
                    break;
            }*/

            logger.error(L"Check format failed for trigger '" + trigger.label() + L"'.");
        }
    }

    void ClipboardActionView::AddActions(const winrt::Windows::Foundation::IInspectable& inspectable)
    {
        AddActions(inspectable.as<win::IVector<win::IVector<winrt::hstring>>>());
    }

    void ClipboardActionView::AddActions(const actions_t actions)
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

    void ClipboardActionView::EditAction(const uint32_t& pos, const winrt::hstring& _label, const winrt::hstring& _format, const winrt::hstring& _regex, const bool& _enabled)
    {
        auto label = std::wstring(_label);
        auto format = std::wstring(_format);
        auto regexString = std::wstring(_regex);

        auto& action = actions[pos]; // Semi-blind index access.
        auto oldLabel = hstring(action.label());
        action.format(format);
        action.label(label);
        action.regex(boost::wregex(regexString));

        // Reload triggers:
        for (uint32_t i = 0; i < TriggersGridView().Items().Size(); i++)
        {
            auto button = TriggersGridView().Items().GetAt(i).try_as<ui::HyperlinkButton>();
            if (button && button.Content().as<hstring>() == oldLabel)
            {
                button.Content(box_value(_label));
                ui::ToolTipService::SetToolTip(button, box_value(std::vformat(_format, std::make_wformat_args(_text))));

                break;
            }
        }
    }

    bool ClipboardActionView::IndexOf(uint32_t& pos, const winrt::hstring& _label)
    {
        auto label = std::wstring(_label);
        for (size_t i = 0; i < actions.size(); i++)
        {
            if (actions[i].label() == label)
            {
                pos = i;
                return true;
            }
        }
        return false;
    }

    winrt::async ClipboardActionView::StartTour()
    {
        visualStateManager.goToState(OptionsOpenState);

        RootGridTeachingTip().IsOpen(true);

        auto func = [ptr = &teachingTipsWaitFlag]() -> void
        {
            ptr->wait(false);
            ptr->clear();
        };

        co_await concurrency::create_task(std::move(func));
    }


    void ClipboardActionView::UserControl_Loading(ui::FrameworkElement const&, win::IInspectable const&)
    {
        std::sort(actions.begin(), actions.end(), [](auto&& a, auto&& b) -> bool
        {
            return a.label().size() < b.label().size();
        });

        // Create buttons for triggers:
        for (auto&& action : actions)
        {
            AddTriggerButton(action);
        }
    }

    void ClipboardActionView::OpenOptionsButton_Click(win::IInspectable const&, ui::RoutedEventArgs const&)
    {
        visualStateManager.switchState(0, true);
    }


    void ClipboardActionView::HyperlinkButton_Click(win::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
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
        else
        {
            logger.debug(L"Missing hstring tag on hyperlink button");
        }
    }

    void ClipboardActionView::FormatLinkButton_Click(win::IInspectable const&, ui::RoutedEventArgs const&)
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

    void ClipboardActionView::RemoveActionButton_Click(win::IInspectable const&, ui::RoutedEventArgs const&)
    {
        e_removed(*this, nullptr);
    }

    void ClipboardActionView::TeachingTip_ButtonClick(ui::Controls::TeachingTip const& sender, win::IInspectable const& args)
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

    void ClipboardActionView::RootGrid_PointerEntered(win::IInspectable const&, ui::Input::PointerRoutedEventArgs const&)
    {
        visualStateManager.goToState(PointerOverState);
        ProtectedCursor(winrt::Microsoft::UI::Input::InputSystemCursor::Create(winrt::Microsoft::UI::Input::InputSystemCursorShape::Hand));
    }

    void ClipboardActionView::RootGrid_PointerExited(win::IInspectable const&, ui::Input::PointerRoutedEventArgs const&)
    {
        visualStateManager.goToState(NormalState);
        ProtectedCursor(winrt::Microsoft::UI::Input::InputSystemCursor::Create(winrt::Microsoft::UI::Input::InputSystemCursorShape::Arrow));
    }

    void ClipboardActionView::RootGrid_PointerPressed(win::IInspectable const&, ui::Input::PointerRoutedEventArgs const& e)
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

    void ClipboardActionView::RootGrid_PointerReleased(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e)
    {
        // TODO: Implement or remove.
    }

    void ClipboardActionView::EditActionButton_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {

    }


    void ClipboardActionView::AddTriggerButton(clip::ClipboardTrigger& trigger)
    {
        ui::HyperlinkButton hyperlinkButton{};
        hyperlinkButton.Content(box_value(trigger.label()));
        hyperlinkButton.Tag(box_value(trigger.label()));
        hyperlinkButton.Click({ this, &ClipboardActionView::HyperlinkButton_Click });

        ui::ToolTipService::SetToolTip(hyperlinkButton, box_value(std::vformat(trigger.format(), std::make_wformat_args(_text))));

        //TriggersGridView().Items().Append(hyperlinkButton);
        TriggersGridView().Items().InsertAt(0, hyperlinkButton);
    }
}
