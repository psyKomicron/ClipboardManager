#include "pch.h"
#include "ToastNotificationHandler.hpp"

#include <src/notifs/win_toasts.hpp>

namespace clipmgr
{
    notifs::ToastNotificationHandler::ToastNotificationHandler()
    {
        static std::once_flag once_flag{};
        std::call_once(once_flag, [this]()
        {
            initializeCompat();
        });
    }

    notifs::ToastNotificationHandler& notifs::ToastNotificationHandler::getDefault()
    {
        static notifs::ToastNotificationHandler instance{};
        return instance;
    }

    void notifs::ToastNotificationHandler::registerAction(const std::wstring& actionName, const std::function<void(ToastNotificationAction)>& callback)
    {
        callbackMap.insert({ actionName, callback });
    }


    void notifs::ToastNotificationHandler::findAction(std::wstring args)
    {
        if (args.empty())
        {
            // TODO: Call the "toast activated" callback.
        }
        else
        {
            // action={}&[parameters]
            auto index = args.find_first_of(L'=');
            auto name = args.substr(0, index);
            if (callbackMap.contains(name))
            {
                ToastNotificationAction action{};
                action.load(args);
                callbackMap[name](std::move(action));
            }
        }
    }

    void notifs::ToastNotificationHandler::initializeCompat()
    {
        logger.info(L"Ensuring that DesktopNotificationManagerCompat has been registered.");
        try
        {
            clipmgr::utils::toasts::compat::DesktopNotificationManagerCompat::Register(L"psykomicron.ClipboardManagerV2", L"ClipboardManager", L"");
            clipmgr::utils::toasts::compat::DesktopNotificationManagerCompat::OnActivated([this](clipmgr::utils::toasts::compat::DesktopNotificationActivatedEventArgsCompat args)
            {
                for (auto&& item : args.UserInput())
                {
                    logger.debug(L"User input: " + std::wstring(item.Key() + item.Value()));
                }
                findAction(args.Argument());
            });
            registered = true;

            logger.info(L"DesktopNotificationManagerCompat successfully registered for toast notifications.");
        }
        catch (const winrt::hresult_error& err)
        {
            registered = false;
            logger.error(L"DesktopNotificationManagerCompat failed to register: " + std::wstring(err.message()));
        }
    }
}

namespace clipmgr
{
    std::map<std::wstring, std::wstring> notifs::ToastNotificationAction::parameters()
    {
        return _parameters;
    }

    void notifs::ToastNotificationAction::load(std::wstring string)
    {
        const wchar_t delimiter{ L'&' };
        size_t pos = std::wstring::npos;
        size_t offset = 0;
        while ((pos = string.find(delimiter, offset)) != std::wstring::npos)
        {
            std::wstring sub = string.substr(offset, pos);
            offset = ++pos;

            auto splitIndex = sub.find(L'=');
            auto first = sub.substr(0, splitIndex);
            auto second = sub.substr(splitIndex + 1);
            _parameters[first] = second;
        }

        if (offset < string.size())
        {
            auto splitIndex = string.find(L'=', offset);
            auto first = string.substr(offset, splitIndex - offset);
            auto second = string.substr(splitIndex + 1);
            _parameters[first] = second;
        }
    }
}
