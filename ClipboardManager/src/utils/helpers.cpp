#include <pch.h>
#include "helpers.hpp"

#include <winrt/Microsoft.UI.Xaml.Data.h>
#include <winrt/Windows.ApplicationModel.Resources.h>

#include <boost/regex.hpp>

#include <Shlwapi.h>
#include <ShlObj.h>

#include <format>
#include <string>
#include <iostream>
#include <locale>
#include <codecvt>


namespace clip::utils
{
    winrt::Microsoft::UI::Windowing::AppWindow getCurrentAppWindow()
    {
        auto hwnd = GetActiveWindow();
        return getCurrentAppWindow(hwnd);
    }

    winrt::Microsoft::UI::Windowing::AppWindow getCurrentAppWindow(const HWND& hwnd)
    {
        return winrt::Microsoft::UI::Windowing::AppWindow::GetFromWindowId(winrt::Microsoft::UI::GetWindowIdFromWindow(hwnd));
    }

    bool pathExists(const std::filesystem::path& path)
    {
        return PathFileExistsW(path.wstring().c_str());
    }

    managed_file_handle createFile(const std::filesystem::path& path)
    {
        auto handle = CreateFileW(path.wstring().c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_NEW, 0, nullptr);

        if (handle == INVALID_HANDLE_VALUE)
        {
            throw std::runtime_error(std::format("Failed to create file: '{}'", path.string()));
        }

        return managed_file_handle(handle);
    }

    void createDirectory(const std::filesystem::path& path)
    {
        if (!CreateDirectoryW(path.wstring().c_str(), nullptr))
        {
            throw std::runtime_error(std::format("Failed to create directory (CreateDirectoryW returned false): '{}'", path.string()));
        }
    }

    std::optional<std::filesystem::path> tryGetKnownFolderPath(const GUID& knownFolderId)
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

    std::optional<winrt::hstring> getNamedResource(const winrt::hstring& name)
    {
        try
        {
            winrt::Windows::ApplicationModel::Resources::ResourceLoader resLoader{};
            return resLoader.GetString(name);
        }
        catch (winrt::hresult_error)
        {
            std::wcerr << L"'getResource' Failed to instanciate or get string from resources." << std::endl;
            return {};
        }
    }

    WindowInfo* getWindowInfo(const HWND& windowHandle)
    {
        return reinterpret_cast<WindowInfo*>(::GetWindowLongPtr(windowHandle, GWLP_USERDATA));
    }

    std::wstring to_wstring(const std::string& string)
    {
        std::wstring wstring{};
        wstring.resize(string.size(), L'\0');
        if (MultiByteToWideChar(CP_UTF8, 0, string.c_str(), string.size(), &wstring[0], string.size()) > 0)
        {
            return wstring;
        }
        else
        {
            return std::wstring();
        }
    }

    std::string to_string(const std::wstring& wstring)
    {
        std::string string{};
        string.resize(wstring.size());

        return WideCharToMultiByte(CP_UTF8, 0, wstring.c_str(), wstring.size(), &string[0], string.size(), NULL, NULL) > 0
            ? string
            : std::string();
    }
}

namespace clip::utils
{
    managed_dispatcher_queue_controller::managed_dispatcher_queue_controller(const winrt::Microsoft::UI::Dispatching::DispatcherQueueController& controller)
    {
        dispatcherQueueController = controller;
    }

    managed_dispatcher_queue_controller::~managed_dispatcher_queue_controller()
    {
        dispatcherQueueController.ShutdownQueue();
    }
}

namespace clip::utils
{
    managed_file_handle::managed_file_handle(const HANDLE& fileHandle)
    {
        handle = fileHandle;
    }

    managed_file_handle::~managed_file_handle()
    {
        close();
    }

    void managed_file_handle::close()
    {
        if (!handleClosed && !invalid())
        {
            CloseHandle(handle);
            handle = nullptr;
            handleClosed = true;
        }
    }

    bool managed_file_handle::invalid() const
    {
        return handle == INVALID_HANDLE_VALUE || handle == nullptr;
    }
}

namespace clip::utils
{
    PropChangedEventArgs::PropChangedEventArgs(const std::source_location& sourceLocation) :
        winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(getCallerName(sourceLocation))
    {
    }

    winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs PropChangedEventArgs::create(std::source_location sourceLocation)
    {
        return winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(getCallerName(sourceLocation));
    }

    std::wstring PropChangedEventArgs::getCallerName(const std::source_location& sourceLocation)
    {
        const boost::regex functionExtractor{ R"(void __cdecl ([A-z]*::)*([A-z]*)\(.*\))" };
        boost::cmatch match{};
        std::ignore = boost::regex_match(sourceLocation.function_name(), match, functionExtractor);
        std::wstring functionName = clip::utils::to_wstring(match[2]);

        return functionName;
    }
}
