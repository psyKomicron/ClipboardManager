#include "pch.h"
#include "StartupTask.hpp"

#include "helpers.hpp"

#include <iostream>
#include <functional>
#include <memory>

bool clipmgr::utils::StartupTask::isTaskRegistered()
{
    //RegCreateKeyW(HKEY_CURRENT_USER)
    auto lresult = RegGetValueW(
        hKey, 
        regAutoRunKey.c_str(),
        regKeyName.c_str(),
        RRF_RT_REG_SZ, 
        nullptr, 
        nullptr, 
        nullptr);

    if (lresult != ERROR_SUCCESS)
    {
        std::wcerr << L"[StartupTask]   Startup task for 'ClipboardManagerV2AutoLaunch' is not registered: " << lresult << std::endl;
    }

    return lresult == ERROR_SUCCESS;
}

void clipmgr::utils::StartupTask::set()
{
    if (!isTaskRegistered())
    {
        std::wcout << L"[StartupTask]   Setting up auto-run registry (" << regAutoRunKey << L"). Key name: " << regKeyName << std::endl;

        HKEY subKey = nullptr;
        DWORD dwDisposition{};
        if (RegCreateKeyExW(hKey, regAutoRunKey.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, nullptr, &subKey, &dwDisposition) == ERROR_SUCCESS)
        {
            clipmgr::utils::managed_resource<HKEY, std::function<LSTATUS(HKEY)>> hKeyPtr2{ subKey, RegCloseKey };
            
            auto startupCommandLine = getStartupCommandLine();
            
            auto result = RegSetValueExW(
                subKey,
                regKeyName.c_str(),
                0,
                regExpectedType,
                reinterpret_cast<const BYTE*>(startupCommandLine.c_str()),
                static_cast<DWORD>(startupCommandLine.length() + 1) * sizeof(std::wstring::value_type));

            if (result != ERROR_SUCCESS)
            {
                std::wcout << L"[StartupTask]   Failed to create auto-run registry entry " << std::quoted(regKeyName) << L" (" << regAutoRunKey << L")" << L": " << result << std::endl;
            }
            else
            {
                std::wcout << L"[StartupTask]   Created auto-run registry entry." << std::endl;
            }
        }
    }
    else
    {
        std::wcout << L"[StartupTask]   Application is already added to the auto-run list." << std::endl;
    }
}

void clipmgr::utils::StartupTask::remove()
{
    if (isTaskRegistered())
    {
        HKEY subKey = nullptr;
        DWORD dwDisposition{};
        if (RegOpenKeyExW(hKey, regAutoRunKey.c_str(), 0, KEY_SET_VALUE, &subKey) == ERROR_SUCCESS)
        {
            clipmgr::utils::managed_resource<HKEY, std::function<LSTATUS(HKEY)>> hKeyPtr2{ subKey, RegCloseKey };

            auto result = RegDeleteValueW(subKey, regKeyName.c_str());

            if (result != ERROR_SUCCESS)
            {
                std::wcout << L"[StartupTask]   Failed to delete auto-run registry entry " << std::quoted(regKeyName) << L" (" << regAutoRunKey << L")" << L": " << result << std::endl;
            }
            else
            {
                std::wcout << L"[StartupTask]   Deleted auto-run registry entry." << std::endl;
            }
        }
    }
}

std::wstring clipmgr::utils::StartupTask::getStartupCommandLine()
{
    auto&& executablePath = getModulePath();
    return std::vformat(L"\"{}\" -as", std::make_wformat_args(executablePath));
}

std::wstring clipmgr::utils::StartupTask::getModulePath()
{
    wchar_t* ptr{};
    auto err = _get_wpgmptr(&ptr);
    return std::wstring(ptr);


    uint32_t requiredSize = GetModuleFileNameW(nullptr, nullptr, 0);
    std::wstring path{};
    path.resize(requiredSize, L'?');
    GetModuleFileNameW(nullptr, path.data(), static_cast<uint32_t>(path.size()));
    return path;
}
