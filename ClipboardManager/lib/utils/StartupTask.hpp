#pragma once
namespace clip::utils
{
    class StartupTask
    {
    public:
        StartupTask() = default;

        bool isTaskRegistered();
        void set();
        void remove();

    private:
        const std::wstring regAutoRunKey{ LR"(SOFTWARE\Microsoft\Windows\CurrentVersion\Run)" };
        const std::wstring regKeyName{ L"ClipboardManagerV2AutoLaunch" };
        const HKEY hKey = HKEY_CURRENT_USER;
        const uint32_t regExpectedType = REG_SZ;

        std::wstring getStartupCommandLine();
        std::wstring getModulePath();
    };
}

