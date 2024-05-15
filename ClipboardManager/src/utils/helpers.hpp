#pragma once
#include <winrt/Microsoft.UI.Windowing.h>

#include <filesystem>
#include <functional>

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

    template<typename T, typename DeleterT>
    class managed_resource
    {
    public:
        managed_resource() = default;

        managed_resource(const T& value, const DeleterT& closer):
            value{value},
            closer{closer},
            hasValue{true}
        {
        }

        managed_resource(managed_resource&) = delete;

        ~managed_resource()
        {
            if (hasValue)
            {
                closer(value);
            }
        }

        T& get()
        {
            return value;
        }

        T release()
        {
            hasValue = false;
            return value;
        }

    private:
        bool hasValue = false;
        T value{};
        DeleterT closer{};
    };

    winrt::Microsoft::UI::Windowing::AppWindow getCurrentAppWindow();
    
    winrt::Microsoft::UI::Windowing::AppWindow getCurrentAppWindow(const HWND& hwnd);
    
    bool pathExists(const std::filesystem::path& path);
    
    managed_file_handle createFile(const std::filesystem::path& path);

    void createDirectory(const std::filesystem::path& path);

    std::optional<std::filesystem::path> tryGetKnownFolderPath(const GUID& knownFolderId);
}
