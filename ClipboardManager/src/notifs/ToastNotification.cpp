#include "pch.h"
#include "ToastNotification.hpp"

#include "src/Settings.hpp"
#include "src/notifs/win_toasts.hpp"

#include <winrt/Windows.UI.Notifications.h>
#include <winrt/Windows.Data.Xml.Dom.h>

#include <atomic>
#include <iostream>

namespace winrt
{
    using namespace winrt::Windows::UI::Notifications;
    using namespace winrt::Windows::Data::Xml::Dom;
}

bool clip::notifs::ToastNotification::registered = false;

clip::notifs::ToastNotification::ToastNotification()
{
    static std::once_flag once_flag{};
    std::call_once(once_flag, [this]()
    {
        ensureToastManagerRegistered();
    });
}

void clip::notifs::ToastNotification::addText(const std::wstring& text)
{
    textElements.push_back(text);
}

void clip::notifs::ToastNotification::setTitle(const std::wstring& title)
{
    this->title = title;
}

void clip::notifs::ToastNotification::addButton(const std::wstring& content, const std::wstring& tag)
{
    buttonElements.push_back({ content, tag });
}

bool clip::notifs::ToastNotification::tryAddButtons(const std::vector<std::pair<std::wstring, std::wstring>>& buttons)
{
    if (buttons.size() > 5)
    {
        return false;
    }

    for (auto&& pair : buttons)
    {
        auto& content = pair.first;
        auto& tag = pair.second;

        buttonElementsTextLength += content.size();
        if (buttonElementsTextLength > buttonElementsTextMaxLength)
        {
            buttonElements.clear();
            return false;
        }

        addButton(content, tag);
    }

    return true;
}

void clip::notifs::ToastNotification::send()
{
    send(NotificationDurationType::Default, NotificationScenarioType::Default, NotificationSoundType::Default);
}

void clip::notifs::ToastNotification::send(const NotificationDurationType& durationType, const NotificationScenarioType& scenarioType, const NotificationSoundType& soundType)
{
    if (!registered)
    {
        throw winrt::hresult_access_denied(L"Application isn't registered to send toast notifications.");
    }

    winrt::XmlDocument doc{};
    doc.LoadXml(LR"(
    <toast>
        <visual>
            <binding template="ToastGeneric">{}</binding>
        </visual>
        <actions>
        </actions>
    </toast>)");

    auto&& toast = doc.SelectSingleNode(L"/toast");
    if (durationType != NotificationDurationType::Default)
    {
        auto attribute = doc.CreateAttribute(L"duration");
        attribute.Value(durationType == NotificationDurationType::Short ? L"short" : L"long");
        toast.Attributes().SetNamedItem(attribute);
    }

    if (scenarioType != NotificationScenarioType::Default)
    {
        auto attribute = doc.CreateAttribute(L"scenario");
        switch (scenarioType)
        {
            case NotificationScenarioType::Reminder:
                attribute.Value(L"reminder");
                break;
            case NotificationScenarioType::Alarm:
                attribute.Value(L"alarm");
                break;
            case NotificationScenarioType::IncomingCall:
                attribute.Value(L"incomingCall");
                break;
            case NotificationScenarioType::Urgent:
                attribute.Value(L"urgent");
                break;
        }

        toast.Attributes().SetNamedItem(attribute);
    }

    if (soundType != NotificationSoundType::Default)
    {
        auto audio = doc.CreateElement(L"audio");

        if (soundType != NotificationSoundType::Silent)
        {
            std::wstring winSound = L"ms-winsoundevent:Notification.";
            auto src = doc.CreateAttribute(L"src");
            switch (soundType)
            {
                case NotificationSoundType::InstantMessage:
                    winSound += L"IM";
                    break;
                case NotificationSoundType::Mail:
                    winSound += L"Mail";
                    break;
                case NotificationSoundType::Reminder:
                    winSound += L"Reminder";
                    break;
                case NotificationSoundType::SMS:
                    winSound += L"SMS";
                    break;
            }
            src.Value(winSound);
            audio.Attributes().SetNamedItem(src);
        }
        else
        {
            auto silent = doc.CreateAttribute(L"silent");
            silent.Value(L"true");
            audio.Attributes().SetNamedItem(silent);
        }

        toast.AppendChild(audio);
    }

    auto&& binding = doc.SelectSingleNode(L"/toast/visual/binding");
    auto text = doc.CreateElement(L"text");
    text.InnerText(title);
    binding.AppendChild(text);

    for (auto&& textElement : textElements)
    {
        auto text = doc.CreateElement(L"text");
        text.InnerText(textElement);
        binding.AppendChild(text);
    }

    if (!buttonElements.empty())
    {
        auto&& actions = doc.SelectSingleNode(L"/toast/actions");
        for (auto&& buttonElement : buttonElements)
        {
            auto action = doc.CreateElement(L"action");
            action.SetAttribute(L"content", buttonElement.first);
            //action.SetAttribute(L"arguments", std::format(L"action={}", buttonElement.second));
            action.SetAttribute(L"arguments", buttonElement.second);
            actions.AppendChild(action);
        }
    }
    else
    {

        // TODO: Dont forget to change the text (inner text) of the launch node.
        /*auto&& toast = doc.SelectSingleNode(L"/toast");
        auto launch = doc.CreateElement(L"launch");
        launch.InnerText(L"asldknfslkdnfsldknfsdf");*/
    }

    winrt::ToastNotification notification{ doc };
    auto toastNotifier = clip::notifs::toasts::compat::DesktopNotificationManagerCompat::CreateToastNotifier();
    toastNotifier.Show(notification);
}


void clip::notifs::ToastNotification::ensureToastManagerRegistered()
{
    logger.info(L"Ensuring that DesktopNotificationManagerCompat has been registered.");
    try
    {
        clip::notifs::toasts::compat::DesktopNotificationManagerCompat::Register(
            L"psykomicron.ClipboardManagerV2", L"ClipboardManager", L""
        );

        logger.info(L"DesktopNotificationManagerCompat successfully registered for toast notifications.");

        registered = true;
    }
    catch (const winrt::hresult_error& err)
    {
        registered = false;
        logger.error(L"DesktopNotificationManagerCompat failed to register: " + std::wstring(err.message()));
    }
}
