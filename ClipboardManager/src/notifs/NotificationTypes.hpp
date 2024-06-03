#pragma once
#include <cstdint>

namespace clipmgr::notifs
{
    enum class NotificationDuration : int
    {
        Default = 0,
        Short = 1,
        Long = 2
    };

    enum class NotificationSound : int
    {
        Default,
        InstantMessage,
        Mail,
        Reminder,
        SMS,
        Silent
    };

    enum class NotificationScenario : int
    {
        Default,
        Reminder,
        Alarm,
        IncomingCall,
        Urgent
    };
}

