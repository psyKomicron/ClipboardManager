#pragma once
#include "src/utils/Logger.hpp"

#include <thread>
#include <optional>
#include <functional>

namespace clipmgr
{
    class HotKey
    {
    public:
        using callback_t = std::function<void()>;

        HotKey(const uint32_t& modifier, const wchar_t& key);
        ~HotKey();

        void startListening(const callback_t& callback);
        void stopListening();
        void editHotKey(const uint32_t& modifier, const wchar_t& key);

        void wait();

        HotKey& operator=(HotKey&& left) noexcept;

    private:
        const clipmgr::utils::Logger logger{ L"HotKey" };
        wchar_t key;
        uint32_t modifier;
        std::jthread keyboardListenerThread{};
        std::atomic<uint32_t> id{};
        std::atomic_bool listening = false;
        std::atomic_bool hotKeyRegistered{};
        std::atomic_flag flag{};
        callback_t callback{};

        void listener();
    };
}

