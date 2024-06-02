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

bool clipmgr::ToastNotification::registered = false;

clipmgr::ToastNotification::ToastNotification()
{
    static std::once_flag once_flag{};
    std::call_once(once_flag, [this]()
    {
        ensureToastManagerRegistered();
    });
}

bool clipmgr::ToastNotification::notificationsAllowed()
{
    clipmgr::Settings settings{};
    return settings.get<bool>(L"NotificationsAllowed").value_or(false);;
}

void clipmgr::ToastNotification::addText(const std::wstring& text)
{
    textElements.push_back(text);
}

void clipmgr::ToastNotification::setTitle(const std::wstring& title)
{
    this->title = title;
}

void clipmgr::ToastNotification::addButton(const std::wstring& content, const std::wstring& tag)
{
    buttonElements.push_back({ content, tag });
}

bool clipmgr::ToastNotification::tryAddButtons(const std::vector<std::pair<std::wstring, std::wstring>>& buttons)
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

void clipmgr::ToastNotification::send()
{
    if (!registered)
    {
        throw winrt::hresult_access_denied(L"Application isn't registered to send toast notifications.");
    }
#ifdef _DEBUG
#else
    if (!notificationsAllowed())
    {
        logger.info(L"Notifications are not allowed, send() will do nothing.");
        return;
    }
#endif // _DEBUG

    winrt::XmlDocument doc{};
    doc.LoadXml(LR"(
    <toast scenario="urgent">
        <visual>
            <binding template="ToastGeneric">{}</binding>
        </visual>
        <actions>
        </actions>
        <audio src="ms-winsoundevent:Notification.SMS"/>
    </toast>)");
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
            action.SetAttribute(L"arguments", std::format(L"action={}", buttonElement.second));
            actions.AppendChild(action);
        }
    }
    else
    {
        // TODO: Dont forget to change the text (inner text) of the launch node.
        auto&& toast = doc.SelectSingleNode(L"/toast");
        auto launch = doc.CreateElement(L"launch");
        launch.InnerText(L"asldknfslkdnfsldknfsdf");
    }

    winrt::ToastNotification notification{ doc };
    auto toastNotifier = clipmgr::utils::toasts::compat::DesktopNotificationManagerCompat::CreateToastNotifier();
    toastNotifier.Show(notification);
}


void clipmgr::ToastNotification::ensureToastManagerRegistered()
{
    logger.info(L"Ensuring that DesktopNotificationManagerCompat has been registered.");
    try
    {
        clipmgr::utils::toasts::compat::DesktopNotificationManagerCompat::Register(
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
