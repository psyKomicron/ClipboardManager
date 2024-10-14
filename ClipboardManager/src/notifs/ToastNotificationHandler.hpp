#pragma once
#include <src/utils/Logger.hpp>

#include <map>
#include <functional>

namespace clip::notifs
{
    class ToastNotificationHandler
    {
    public:
        ToastNotificationHandler();

        void registerActionCallback(const std::function<void(std::wstring)>& callback);
        void registerActivatedCallback(const std::function<void()>& callback);

    private:
        clip::utils::Logger logger{ L"ToastNotificationHandler" };
        bool registered = false;
        std::function<void(std::wstring)> buttonClickedCallback{};
        std::function<void()> toastClickedCallback{};

        void findAction(std::wstring args);
        void initializeCompat();
    };
}