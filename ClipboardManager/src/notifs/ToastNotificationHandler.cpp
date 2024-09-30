#include "pch.h"
#include "ToastNotificationHandler.hpp"

#include <src/notifs/win_toasts.hpp>

namespace clip
{
    notifs::ToastNotificationHandler::ToastNotificationHandler()
    {
        static std::once_flag once_flag{};
        std::call_once(once_flag, [this]()
        {
            initializeCompat();
        });
    }

    void notifs::ToastNotificationHandler::registerActionCallback(const std::function<void(std::wstring)>& callback)
    {
        buttonClickedCallback = callback;
    }

    void notifs::ToastNotificationHandler::registerActivatedCallback(const std::function<void()>& callback)
    {
        toastClickedCallback = callback;
    }


    void notifs::ToastNotificationHandler::findAction(std::wstring args)
    {
        if (args.empty())
        {
            // TODO: Call the "toast activated" callback.
            toastClickedCallback();
        }
        else
        {
            buttonClickedCallback(args);
        }
    }

    void notifs::ToastNotificationHandler::initializeCompat()
    {
        logger.info(L"Ensuring that DesktopNotificationManagerCompat has been registered.");
        try
        {
            clip::notifs::toasts::compat::DesktopNotificationManagerCompat::Register(L"psykomicron.ClipboardManagerV2", L"ClipboardManager", L"");
            clip::notifs::toasts::compat::DesktopNotificationManagerCompat::OnActivated([this](clip::notifs::toasts::compat::DesktopNotificationActivatedEventArgsCompat args)
            {
                try
                {
                    findAction(args.Argument());
                }
                catch (winrt::hresult_error err)
                {
                    logger.error(err.message().data());
                }
                catch (...)
                {
                    logger.error(L"Unspecified error.");
                }
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