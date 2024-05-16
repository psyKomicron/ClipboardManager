#include "pch.h"
#include "Settings.hpp"

#include <wil/registry_helpers.h>

#include <iostream>

clipmgr::Settings::Settings()
{
    hKey = wil::reg::create_unique_key(HKEY_CURRENT_USER, (L"SOFTWARE\\" + applicationName).c_str(), wil::reg::key_access::readwrite);
    logger.debug(L"Opened registry node for application settings.");
}

std::optional<std::wstring> clipmgr::Settings::getString(const std::wstring& key)
{
    return wil::reg::try_get_value_string(hKey.get(), key.c_str());
}

void clipmgr::Settings::insert(const std::wstring& key, const std::wstring& value)
{
    wil::reg::set_value(hKey.get(), key.c_str(), value.c_str());
}

void clipmgr::Settings::insert(const std::wstring& key, const bool& value)
{
    wil::reg::set_value_dword(hKey.get(), key.c_str(), value ? 1 : 0);
}

std::vector<std::pair<std::wstring, clipmgr::reg_types>> clipmgr::Settings::getAll()
{
    std::vector<std::pair<std::wstring, clipmgr::reg_types>> entries{};

    logger.info(std::format(L"Retreiving all settings. {} values, {} subkeys.", 
        wil::reg::get_child_value_count(hKey.get()),
        wil::reg::get_child_key_count(hKey.get())));

    // Iterate sub keys.
    {
        auto range = wil::make_range(wil::reg::key_iterator(hKey.get()), wil::reg::key_iterator());
        for (auto&& key : range)
        {
            entries.push_back({ key.name, L"" });
        }
    }

    // Iterate values.
    auto range = wil::make_range(wil::reg::value_iterator(hKey.get()), wil::reg::value_iterator());
    for (auto&& key : range)
    {
        key.name;
        reg_types variant{};
        switch (key.type)
        {
            case REG_SZ:
                variant.emplace<0>(getValue<std::wstring>(key.name).value());
                break;
            case REG_MULTI_SZ:
                variant.emplace<1>(std::vector<std::wstring>());
                break;
            case REG_DWORD:
                variant.emplace<2>(getValue<uint32_t>(key.name).value());
                break;
            case REG_QWORD:
                variant.emplace<3>(getValue<uint64_t>(key.name).value());
                break;
        }

        entries.push_back(std::make_pair(key.name, variant));
    }

    return entries;
}

void clipmgr::Settings::clear()
{
    clearKey(hKey.get());
}


void clipmgr::Settings::clearKey(HKEY hkey)
{
    auto subKeys = wil::make_range(wil::reg::key_iterator(hKey.get()), wil::reg::key_iterator());
    auto keyValues = wil::make_range(wil::reg::value_iterator(hKey.get()), wil::reg::value_iterator());

    for (auto&& value : keyValues)
    {
        RegDeleteValueW(hKey.get(), value.name.c_str());
    }

    for (auto&& subKey : subKeys)
    {
        auto uniqueKey = wil::reg::create_unique_key(hKey.get(), subKey.name.c_str(), wil::reg::key_access::readwrite);
        clearKey(uniqueKey.get());
    }
}
