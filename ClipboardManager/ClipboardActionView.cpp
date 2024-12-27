#include "pch.h"
#include "ClipboardActionView.h"
#if __has_include("ClipboardActionView.g.cpp")
#include "ClipboardActionView.g.cpp"
#endif

#include "src/Settings.hpp"
#include "src/utils/Launcher.hpp"
#include "src/res/strings.h"
#include "Resource.h"

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
    clip::utils::ResLoader ClipboardActionView::resLoader{};

    ClipboardActionView::ClipboardActionView()
    {
        visualStateManager.initializeStates(
            {
                optionsClosedState,
                optionsOpenState
            });
        
        _timestamp = winrt::clock::now();
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

    Windows::Foundation::DateTime ClipboardActionView::Timestamp()
    {
        return _timestamp;
    }

    void ClipboardActionView::Timestamp(const Windows::Foundation::DateTime& value)
    {
        _timestamp = value;
    }

    winrt::event_token ClipboardActionView::Removed(const event_removed_t& handler)
    {
        return e_removed.add(handler);
    }

    void ClipboardActionView::Removed(const winrt::event_token& token)
    {
        e_removed.remove(token);
    }

    void ClipboardActionView::AddAction(const winrt::hstring& label, const winrt::hstring& format, const winrt::hstring& regex, 
                                        const bool& enabled, const bool& useRegexMatchResults, const bool& ignoreCase)
    {
        try
        {
            auto trigger = clip::ClipboardTrigger(
                std::wstring(label), 
                std::wstring(format), 
                boost::wregex(std::wstring(regex), ignoreCase ? boost::regex_constants::icase : boost::regex_constants::basic), 
                enabled);
            trigger.useRegexMatchResults(useRegexMatchResults);

            trigger.checkFormat();
            triggers.push_back(std::move(trigger));
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

            logger.error(L"Check format failed for trigger '" + std::wstring(label) + L"'.");
        }
    }

    void ClipboardActionView::EditAction(const uint32_t& pos, const winrt::hstring& _label, const winrt::hstring& _format, const winrt::hstring& _regex, 
                                         const bool& _enabled, const bool& useRegexMatchResults, const bool& ignoreCase)
    {
        auto label = std::wstring(_label);
        auto format = std::wstring(_format);
        auto regexString = std::wstring(_regex);

        auto& trigger = triggers[pos]; // Semi-blind index access.
        auto oldLabel = hstring(trigger.label());
        trigger.format(format);
        trigger.label(label);
        trigger.regex(boost::wregex(regexString));
        trigger.useRegexMatchResults(useRegexMatchResults);

        // Reload triggers:
        for (uint32_t i = 0; i < TriggersGridView().Items().Size(); i++)
        {
            auto button = TriggersGridView().Items().GetAt(i).try_as<ui::HyperlinkButton>();
            if (button && button.Content().as<hstring>() == oldLabel)
            {
                button.Content(box_value(_label));
                ui::ToolTipService::SetToolTip(button, box_value(trigger.formatTrigger(_text.data())));

                break;
            }
        }
    }

    bool ClipboardActionView::IndexOf(uint32_t& pos, const winrt::hstring& _label)
    {
        auto label = std::wstring(_label);
        for (size_t i = 0; i < triggers.size(); i++)
        {
            if (triggers[i].label() == label)
            {
                pos = i;
                return true;
            }
        }
        return false;
    }

    winrt::Windows::Foundation::Collections::IVector<hstring> ClipboardActionView::GetTriggersText()
    {
        auto triggersText = single_threaded_vector<hstring>();
        for (auto&& trigger : triggers)
        {
            triggersText.Append(trigger.label());
        }
        return triggersText;
    }

    winrt::async ClipboardActionView::StartTour()
    {
        visualStateManager.goToState(optionsOpenState);

        RootGridTeachingTip().IsOpen(true);

        auto func = [ptr = &teachingTipsWaitFlag]() -> void
        {
            ptr->wait(false);
            ptr->clear();
        };

        co_await concurrency::create_task(std::move(func));
    }


    void ClipboardActionView::AddTriggerButton(clip::ClipboardTrigger& trigger)
    {
        ui::HyperlinkButton hyperlinkButton{};
        hyperlinkButton.Content(box_value(trigger.label()));
        hyperlinkButton.Tag(box_value(trigger.label()));
        hyperlinkButton.Click({ this, &ClipboardActionView::HyperlinkButton_Click });

        ui::ToolTipService::SetToolTip(hyperlinkButton, 
                                       box_value(trigger.formatTrigger(std::wstring(_text))));

        TriggersGridView().Items().InsertAt(0, hyperlinkButton);
    }

    void ClipboardActionView::CopyLinkToClipboard()
    {
        if (triggers.size() > 0)
        {
            try
            {
                win::DataPackage dataPackage{};
                dataPackage.SetText(triggers[0].formatTrigger(_text.data()));
                dataPackage.Properties().ApplicationName(APP_NAMEW);
                win::Clipboard::SetContent(dataPackage);

                InfoBar().Message(resLoader.getResource(L"Message_CopiedToClipboard").value_or(L"Copied"));
                InfoBar().Severity(ui::InfoBarSeverity::Success);
                InfoBar().IsOpen(true);
            }
            catch (clip::ClipboardTriggerFormatException)
            {
                logger.error(L"Failed to format and add formatted link to clipboard (ClipboardActionView::CopyLinkToClipboard).");

                InfoBar().Message(resLoader.getResource(L"ErrorMessage_FailedToCopyToClipboard").value_or(L"Error"));
                InfoBar().Severity(ui::InfoBarSeverity::Error);
                InfoBar().IsOpen(true);
            }
        }
    }


    void ClipboardActionView::UserControl_Loading(ui::FrameworkElement const&, win::IInspectable const&)
    {
        std::sort(triggers.begin(), triggers.end(), [](auto&& a, auto&& b) -> bool
        {
            return a.label().size() < b.label().size();
        });

        // Create buttons for triggers:
        for (auto&& action : triggers)
        {
            AddTriggerButton(action);
        }
    }

    void ClipboardActionView::OpenOptionsButton_Click(win::IInspectable const&, ui::RoutedEventArgs const&)
    {
        visualStateManager.switchState(0, true);
        if (visualStateManager.getCurrentState(0).name() == optionsOpenState.name())
        {
            OptionsGrid().Scale({ 1, 1, 1 });
        }
        else
        {
            OptionsGrid().Scale({ 0, 0, 0 });
        }
    }

    void ClipboardActionView::HyperlinkButton_Click(win::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        auto&& label = sender.as<ui::Control>().Tag().try_as<hstring>();
        if (label.has_value() && !label.value().empty())
        {
            for (auto&& trigger : triggers)
            {
                if (label.value() == trigger.label())
                {
                    clip::utils::Launcher launcher{};
                    launcher.launch(trigger, std::wstring(_text));

                    break;
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
        CopyLinkToClipboard();
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
        visualStateManager.goToState(pointerOverState);
        ProtectedCursor(winrt::Microsoft::UI::Input::InputSystemCursor::Create(winrt::Microsoft::UI::Input::InputSystemCursorShape::Hand));
    }

    void ClipboardActionView::RootGrid_PointerExited(win::IInspectable const&, ui::Input::PointerRoutedEventArgs const&)
    {
        visualStateManager.goToState(normalState);
        ProtectedCursor(winrt::Microsoft::UI::Input::InputSystemCursor::Create(winrt::Microsoft::UI::Input::InputSystemCursorShape::Arrow));
    }

    void ClipboardActionView::RootGrid_PointerPressed(win::IInspectable const&, ui::Input::PointerRoutedEventArgs const& e)
    {
        if (e.GetCurrentPoint(*this).Properties().IsLeftButtonPressed())
        {
            visualStateManager.goToState(pointerPressedState);

            if (triggers.size() > 0)
            {
                clip::Settings settings{};
                if (settings.get<bool>(L"ClipboardActionClick").value_or(false))
                {
                    auto&& action = triggers[0];
                    clip::utils::Launcher launcher{};
                    launcher.launch(action, std::wstring(_text));
                }
                else
                {
                    CopyLinkToClipboard();
                }
            }
        }

    }
}


