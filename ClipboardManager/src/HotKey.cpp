#include "pch.h"
#include "HotKey.hpp"
// TODO: Find a better implementation using std::jthread.

clip::HotKey::HotKey(const uint32_t& modifier, const wchar_t& key) :
    modifier{ modifier },
    key{ key }
{
}

clip::HotKey::~HotKey()
{
    stopListening();
}

void clip::HotKey::startListening(const callback_t& callback)
{
    this->callback = callback;
    keyboardListenerThread = std::jthread(&HotKey::listener, this);
    flag.wait(false);
    flag.clear();
}

void clip::HotKey::stopListening()
{
    if (hotKeyRegistered)
    {
        UnregisterHotKey(nullptr, id);
    }

    if (flag.test())
    {
        PostThreadMessageW(id, WM_QUIT, 0, 0);
    }

    std::ignore = keyboardListenerThread.request_stop();
}

void clip::HotKey::editHotKey(const uint32_t& modifier_, const wchar_t& key_)
{
    stopListening();
    modifier = modifier_;
    key = key_;
    startListening(callback);
}

void clip::HotKey::wait()
{
    flag.wait(false);
    flag.clear();
}

clip::HotKey& clip::HotKey::operator=(HotKey&& left) noexcept
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


void clip::HotKey::listener()
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
        logger.error(L"Failed to register hotkey.");
    }

    listening = false;
    flag.notify_all();
}
