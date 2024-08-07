#pragma once
#include <src/utils/Logger.hpp>

#include <map>
#include <functional>

namespace clip::notifs
{
    class ToastNotificationAction
    {
    public:
        ToastNotificationAction() = default;

        template<typename T>
        T action();

        template<>
        std::wstring action()
        {
            return _parameters[L"action"];
        }

        std::map<std::wstring, std::wstring> parameters();
        void load(std::wstring string);

    private:
        std::map<std::wstring, std::wstring> _parameters{};
    };

    class ToastNotificationHandler
    {
    public:
        static ToastNotificationHandler& getDefault();

        ToastNotificationHandler(ToastNotificationHandler&&) = delete;
        ToastNotificationHandler(const ToastNotificationHandler&) = delete;
        ToastNotificationHandler operator=(ToastNotificationHandler) = delete;

        void registerAction(const std::wstring& actionName, const std::function<void(ToastNotificationAction)>& callback);

    private:
        clip::utils::Logger logger{ L"ToastNotificationHandler" };
        bool registered = false;
        std::map<std::wstring, std::function<void(ToastNotificationAction)>> callbackMap{};

        ToastNotificationHandler();

        void findAction(std::wstring args);
        void initializeCompat();
    };
}