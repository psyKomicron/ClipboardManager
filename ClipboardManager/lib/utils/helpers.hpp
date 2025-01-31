#pragma once
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Windows.ApplicationModel.DataTransfer.h>
#include <Windows.h>

#include <boost/regex.hpp>

#include <filesystem>
#include <functional>
#include <string>
#include <concepts>

#define check_loaded(b) if (!b) return;

namespace winrt
{
    using namespace winrt::Microsoft::UI;
    using namespace winrt::Microsoft::UI::Dispatching;
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Hosting;
    using namespace winrt::Microsoft::UI::Xaml::Markup;
    using namespace winrt::Microsoft::UI::Windowing;
}

// managed_dispatcher_queue_controller
namespace clip::utils
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

    using reg_key = managed_resource<HKEY, std::function<LSTATUS(HKEY)>>;

    struct WindowInfo
    {
        winrt::DesktopWindowXamlSource desktopWinXamlSrc{ nullptr };
        winrt::event_token takeFocusRequestedToken{};
        HWND lastFocusedWindow{ nullptr };
    };


    class PropChangedEventArgs : public winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs
    {
    public:
        PropChangedEventArgs(const std::source_location& sourceLocation);

        static winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs create(std::source_location sourceLocation = std::source_location::current());

    private:
        std::wstring propName;

        static std::wstring getCallerName(const std::source_location& sourceLocation);
    };
}

// Utils functions
namespace clip::utils
{
    winrt::AppWindow getCurrentAppWindow();
    winrt::AppWindow getCurrentAppWindow(const HWND& hwnd);

    bool pathExists(const std::filesystem::path& path);
    managed_file_handle createFile(const std::filesystem::path& path);
    void createDirectory(const std::filesystem::path& path);
    std::optional<std::filesystem::path> tryGetKnownFolderPath(const GUID& knownFolderId);
    std::optional<winrt::hstring> getNamedResource(const winrt::hstring& name);

    WindowInfo* getWindowInfo(const HWND& windowHandle);

    std::wstring to_wstring(const std::string& string);
    std::string to_string(const std::wstring& string);
    
    template<typename _Elem, typename _Traits>
    auto to_visual_string(std::basic_string<_Elem, _Traits>&& string)
    {
        if (string.empty())
        {
            if constexpr (std::same_as<wchar_t, _Elem>)
            {
                return std::basic_string<_Elem, _Traits>(L"<empty>");
            }
            else
            {
                return std::basic_string<_Elem, _Traits>("empty");
            }
        }
        else
        {
            return string;
        }
    }
}

namespace clip::utils
{
    class clipboard_properties_formatter
    {
    public:
        std::wstring format(winrt::Windows::ApplicationModel::DataTransfer::DataPackageView& content);
    };
}

namespace clip::utils
{
    class ClipboardSourceFinder
    {
    public:
        ClipboardSourceFinder() = default;

        std::wstring findSource(std::wstring_view&& value);

    private:
        std::vector<std::pair<boost::wregex, const std::wstring>> sources{};
    };
}