#include <pch.h>
#include "helpers.hpp"

#include <Shlwapi.h>
#include <ShlObj.h>

#include <format>
#include <string>

winrt::Microsoft::UI::Windowing::AppWindow clipmgr::utils::getCurrentAppWindow()
{
    auto hwnd = GetActiveWindow();
    return getCurrentAppWindow(hwnd);
}

winrt::Microsoft::UI::Windowing::AppWindow clipmgr::utils::getCurrentAppWindow(const HWND& hwnd)
{
    return winrt::Microsoft::UI::Windowing::AppWindow::GetFromWindowId(winrt::Microsoft::UI::GetWindowIdFromWindow(hwnd));
}

bool clipmgr::utils::pathExists(const std::filesystem::path& path)
{
    return PathFileExistsW(path.wstring().c_str());
}

clipmgr::utils::managed_file_handle clipmgr::utils::createFile(const std::filesystem::path& path)
{
    auto handle = CreateFileW(path.wstring().c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_NEW, 0, nullptr);

    if (handle == INVALID_HANDLE_VALUE)
    {
        throw std::runtime_error(std::format("Failed to create file: '{}'", path.string()));
    }

    return clipmgr::utils::managed_file_handle(handle);
}

void clipmgr::utils::createDirectory(const std::filesystem::path& path)
{
    if (!CreateDirectoryW(path.wstring().c_str(), nullptr))
    {
        throw std::runtime_error(std::format("Failed to create directory (CreateDirectoryW returned false): '{}'", path.string()));
    }
}

std::optional<std::filesystem::path> clipmgr::utils::tryGetKnownFolderPath(const GUID& knownFolderId)
{
    wchar_t* pWstr = nullptr;
    if (SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, nullptr, &pWstr) == S_OK && pWstr != nullptr)
    {
        // RAII pWstr.
        std::unique_ptr<wchar_t, std::function<void(void*)>> ptr{ pWstr, CoTaskMemFree };
        return std::filesystem::path(pWstr);
    }
    else
    {
        return std::optional<std::filesystem::path>();
    }
}

clipmgr::utils::WindowInfo* clipmgr::utils::getWindowInfo(const HWND& windowHandle)
{
    return reinterpret_cast<WindowInfo*>(::GetWindowLongPtr(windowHandle, GWLP_USERDATA));
}


clipmgr::utils::managed_dispatcher_queue_controller::managed_dispatcher_queue_controller(const winrt::Microsoft::UI::Dispatching::DispatcherQueueController& controller)
{
    dispatcherQueueController = controller;
}

clipmgr::utils::managed_dispatcher_queue_controller::~managed_dispatcher_queue_controller()
{
    dispatcherQueueController.ShutdownQueue();
}

clipmgr::utils::managed_file_handle::managed_file_handle(const HANDLE& fileHandle)
{
    handle = fileHandle;
}

clipmgr::utils::managed_file_handle::~managed_file_handle()
{
    close();
}

void clipmgr::utils::managed_file_handle::close()
{
    if (!handleClosed && !invalid())
    {
        CloseHandle(handle);
        handle = nullptr;
        handleClosed = true;
    }
}

bool clipmgr::utils::managed_file_handle::invalid() const
{
    return handle == INVALID_HANDLE_VALUE || handle == nullptr;
}

