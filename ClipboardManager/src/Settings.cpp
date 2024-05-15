#include "pch.h"
#include "Settings.hpp"

#include <iostream>

clipmgr::Settings::Settings()
{
    hKey = wil::reg::create_unique_key(HKEY_CURRENT_USER, applicationName.c_str(), wil::reg::key_access::readwrite);
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

    std::wcout << L"[Settings]   Retreiving all settings. " << wil::reg::get_child_value_count(hKey.get())
        << L" values, " << wil::reg::get_child_key_count(hKey.get()) << L" subkeys." << std::endl;

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
