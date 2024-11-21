#include "pch.h"
#include "HotKey.hpp"

#include "src/utils/helpers.hpp"

namespace clip
{
    HotKey::HotKey(const uint32_t& modifier, const wchar_t& key) :
        modifier{ modifier },
        key{ key }
    {
    }

    HotKey::~HotKey()
    {
        stopListening();
    }

    bool HotKey::registered() const
    {
        return hotKeyRegistered;
    }

    void HotKey::startListening(const callback_t& callback)
    {
        this->callback = callback;
        keyboardListenerThread = std::jthread(&HotKey::listener, this);
        flag.wait(false);
        flag.clear();

        if (!hotKeyRegistered)
        {
            throw std::invalid_argument("Failed to register hotkey.");
        }
    }

    void HotKey::editHotKey(const uint32_t& modifier_, const wchar_t& key_)
    {
        stopListening();
        modifier = modifier_;
        key = key_;
        startListening(callback);
    }

    void HotKey::wait()
    {
        flag.wait(false);
        flag.clear();
    }

    HotKey& HotKey::operator=(HotKey&& left)
    {
        modifier = left.modifier;
        key = left.key;
        hotKeyRegistered = left.hotKeyRegistered.load();
        listening = left.listening.load();
    
        if (hotKeyRegistered && listening)
        {
            startListening(left.callback);
        }

        // Not copying: keyboardListenerThread, id, flag, callback.

        return *this;
    }


    void HotKey::stopListening()
    {
        if (hotKeyRegistered)
        {
            UnregisterHotKey(nullptr, id);
        }

        if (listening)
        {
            PostThreadMessageW(id, WM_QUIT, 0, 0);
            std::ignore = keyboardListenerThread.request_stop();
        }
    }

    void HotKey::listener()
    {
        listening = true;
        flag.test_and_set();

        auto threadId = GetCurrentThreadId();
        id.store(threadId);

        if (RegisterHotKey(nullptr, id, modifier, key))
        {
            hotKeyRegistered = true;
            flag.notify_all();

            logger.info(std::format(L"(id {}) Hot key registered.", threadId));

            MSG message{};
            while (GetMessageW(&message, reinterpret_cast<HWND>(-1), 0, 0))
            {
                if (message.message == WM_HOTKEY)
                {
                    callback();
                }
            }
        }
        else
        {
            unsigned long lastError = GetLastError();
            std::wstring message = clip::utils::to_wstring(std::system_category().message(lastError));
            logger.error(std::vformat(L"(id {}) Failed to register hotkey: {}", std::make_wformat_args(threadId, message)));
        }

        listening = false;
        flag.notify_all();
    }
}
