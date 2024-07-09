#pragma once
#include <cstdint>

namespace clipmgr::notifs
{
    enum class NotificationDurationType : int
    {
        Default = 0,
        Short = 1,
        Long = 2
    };

    enum class NotificationSoundType : int
    {
        Default,
        InstantMessage,
        Mail,
        Reminder,
        SMS,
        Silent
    };

    enum class NotificationScenarioType : int
    {
        Default,
        Reminder,
        Alarm,
        IncomingCall,
        Urgent
    };
}

