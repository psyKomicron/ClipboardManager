#pragma once
#include <src/utils/Logger.hpp>
namespace clipmgr::notifs
{
    class ToastNotificationHandler
    {
    public:
        static ToastNotificationHandler& getDefault();

    private:
        clipmgr::utils::Logger logger{ L"ToastNotificationHandler" };
        bool registered = false;

        ToastNotificationHandler();

        void initializeCompat();
    };
}