#pragma once
#include <winrt/Microsoft.UI.Windowing.h>

#include <filesystem>

namespace clipmgr::utils
{
    class managed_dispatcher_queue_controller
    {
    public:
        managed_dispatcher_queue_controller(const winrt::Microsoft::UI::Dispatching::DispatcherQueueController& controller);
        ~managed_dispatcher_queue_controller();

    private:
        winrt::Microsoft::UI::Dispatching::DispatcherQueueController dispatcherQueueController{ nullptr };
    };

    class managed_file_handle
    {
    public:
        managed_file_handle(const HANDLE& fileHandle);
        ~managed_file_handle();

        void close();
        bool invalid() const;

    private:
        bool handleClosed{};
        HANDLE handle;
    };


    winrt::Microsoft::UI::Windowing::AppWindow getCurrentAppWindow();
    winrt::Microsoft::UI::Windowing::AppWindow getCurrentAppWindow(const HWND& hwnd);
    bool pathExists(const std::filesystem::path& path);
    managed_file_handle createFile(const std::filesystem::path& path);
    void createDirectory(const std::filesystem::path& path);

}
