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

    void HotKey::stopListening()
    {
        if (listening)
        {
            PostThreadMessageW(id, WM_QUIT, 0, 0);
            std::ignore = keyboardListenerThread.request_stop();
        }

        if (hotKeyRegistered)
        {
            UnregisterHotKey(nullptr, id);
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

    HotKey& HotKey::operator=(HotKey&& left) noexcept
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
            auto lastError = GetLastError();
            logger.error(L"Failed to register hotkey: " + clip::utils::to_wstring(std::system_category().message(lastError)));
        }

        listening = false;
        flag.notify_all();
    }
}
