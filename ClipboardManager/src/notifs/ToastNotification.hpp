#pragma once
#include "src/utils/Logger.hpp"

#include <vector>
#include <string>

namespace clipmgr
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

    private:
        static bool registered;
        clipmgr::utils::Logger logger{ L"ToastNotification" };
        std::wstring title{};
        std::vector<std::wstring> textElements{};
        std::vector<std::pair<std::wstring, std::wstring>> buttonElements{};
        const size_t buttonElementsTextMaxLength = (12 * 2);
        size_t buttonElementsTextLength = 0;

        void ensureToastManagerRegistered();
    };
}

