#pragma once
#include "ClipboardAction.hpp"

#include <wil/registry.h>

#include <string>
#include <vector>
#include <variant>
#include <map>

namespace clipmgr
{
    using reg_types = std::variant<std::wstring, std::vector<std::wstring>, uint32_t, uint64_t>;

    enum class RegTypes
    {
        String = 0,
        StringVector = 1,
        Uint32 = 2,
        Uint64 = 3
    };

    class Settings
    {
    public:
        Settings();

        std::optional<std::wstring> getString(const std::wstring& key);

        template<typename T>
        std::optional<T> get(const std::wstring& key)
        {
            return getValue<T>(key);
        }

        void insert(const std::wstring& key, const std::wstring& value);
        void insert(const std::wstring& key, const bool& value);

        std::vector<std::pair<std::wstring, clipmgr::reg_types>> getAll();

    private:
        const std::wstring applicationName = L"ClipboardManagerV2";
        wil::unique_hkey hKey{ nullptr };

        template<typename T>
        std::optional<T> getValue(const std::wstring& key)
        {
            return std::optional<T>();
        }

        template<>
        std::optional<std::wstring> getValue(const std::wstring& key)
        {
            return wil::reg::try_get_value_string(hKey.get(), key.c_str());
        }

        template<>
        std::optional<uint32_t> getValue(const std::wstring& key)
        {
            return wil::reg::try_get_value_dword(hKey.get(), key.c_str());
        }

        template<>
        std::optional<bool> getValue(const std::wstring& key)
        {
            std::optional<uint32_t> dword = wil::reg::try_get_value_dword(hKey.get(), key.c_str());

            if (dword.has_value())
            {
                return dword.value() == 1;
            }
            else
            {
                return std::optional<bool>();
            }
        }
    };
}

