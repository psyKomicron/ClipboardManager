#pragma once
#include "src/utils/Logger.hpp"
#include "src/notifs/NotificationTypes.hpp"

#include <vector>
#include <string>

namespace clip::notifs
{
    class ToastNotification
    {
    public:
        ToastNotification();

        bool notificationsAllowed();

        void addText(const std::wstring& text);
        void setTitle(const std::wstring& title);
        void addButton(const std::wstring& content, const std::wstring& tag);
        bool tryAddButtons(const std::vector<std::pair<std::wstring, std::wstring>>& buttons);

        void send();
        void send(const NotificationDurationType& durationType, const NotificationScenarioType& scenarioType, const NotificationSoundType& soundType);

    private:
        static bool registered;
        clip::utils::Logger logger{ L"ToastNotification" };
        std::wstring title{};
        std::vector<std::wstring> textElements{};
        std::vector<std::pair<std::wstring, std::wstring>> buttonElements{};
        const size_t buttonElementsTextMaxLength = (12 * 2);
        const size_t maxButtonCount = 5;
        size_t buttonElementsTextLength = 0;

        void ensureToastManagerRegistered();
    };
}

