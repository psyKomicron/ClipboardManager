#pragma once
#include "lib/utils/Logger.hpp"

#include <thread>
#include <optional>
#include <functional>

namespace clip
{
    class HotKey
    {
    public:
        using callback_t = std::function<void()>;

        HotKey(const uint32_t& modifier, const wchar_t& key);
        ~HotKey();

        bool registered() const;

        void startListening(const callback_t& callback);
        void editHotKey(const uint32_t& modifier, const wchar_t& key);
        void wait();

        HotKey& operator=(HotKey&& left) noexcept;

    private:
        clip::utils::Logger logger{ L"HotKey" };
        wchar_t key;
        uint32_t modifier;
        std::jthread keyboardListenerThread{};
        std::atomic<uint32_t> id{};
        std::atomic_bool listening = false;
        std::atomic_bool hotKeyRegistered{};
        std::atomic_flag flag{};
        callback_t callback{};

        void stopListening();
        void listener();
    };

    enum class HotKeyModifiers
    {
        // TODO:
    };
}

