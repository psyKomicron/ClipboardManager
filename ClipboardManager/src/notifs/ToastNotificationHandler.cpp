#include "pch.h"
#include "ToastNotificationHandler.hpp"

#include <src/notifs/win_toasts.hpp>


clipmgr::notifs::ToastNotificationHandler::ToastNotificationHandler()
{
    static std::once_flag once_flag{};
    std::call_once(once_flag, [this]()
    {
        initializeCompat();
    });
}

clipmgr::notifs::ToastNotificationHandler& clipmgr::notifs::ToastNotificationHandler::getDefault()
{
    static clipmgr::notifs::ToastNotificationHandler instance{};
    return instance;
}

void clipmgr::notifs::ToastNotificationHandler::initializeCompat()
{
    logger.info(L"Ensuring that DesktopNotificationManagerCompat has been registered.");
    
    try
    {
        clipmgr::utils::toasts::compat::DesktopNotificationManagerCompat::Register(
            L"psykomicron.ClipboardManagerV2", L"ClipboardManager", L""
        );
        registered = true;

        logger.info(L"DesktopNotificationManagerCompat successfully registered for toast notifications.");

        clipmgr::utils::toasts::compat::DesktopNotificationManagerCompat::OnActivated([this](auto&& args)
        {
            logger.info(L"Toast activated: " + args.Argument());
        });
    }
    catch (const winrt::hresult_error& err)
    {
        registered = false;
        logger.error(L"DesktopNotificationManagerCompat failed to register: " + std::wstring(err.message()));
    }
}
