// ClipboardManager.cpp : Defines the entry point for the application.
#include "pch.h"
#include "ClipboardManager.h"

#include "src/utils/helpers.hpp"
#include "src/utils/Console.hpp"
#include "src/Settings.hpp"

#include "App.xaml.h"
#include "MainPage.h"

#include <Microsoft.UI.Dispatching.Interop.h> // For ContentPreTranslateMessage

#include <memory>
#include <iostream>
#include <chrono>

#pragma region Header
#define MAX_LOADSTRING 100

// Global Variables:
WCHAR szTitle[MAX_LOADSTRING];// The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];// the main window class name

constexpr uint32_t InitialWindowWidth = 470;
constexpr uint32_t InitialWindowHeight = 700;

namespace winrt
{
    using namespace winrt::Microsoft::UI;
    using namespace winrt::Microsoft::UI::Dispatching;
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Hosting;
    using namespace winrt::Microsoft::UI::Xaml::Markup;
    using namespace winrt::Microsoft::UI::Windowing;
}

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance);
HWND InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

winrt::DispatcherQueueController initIslandApp();
bool processMessageForTabNav(const HWND& window, MSG& msg);
void createWinUIWindow(clipmgr::utils::WindowInfo* windowInfo, const HWND& windowHandle);
void handleWindowActivation(clipmgr::utils::WindowInfo* windowInfo, const bool& inactive);
#pragma endregion

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int nCmdShow)
{
#ifdef _DEBUG
    clipmgr::utils::Console console{};
#endif

    try
    {
        auto dispatcherQueueController = clipmgr::utils::managed_dispatcher_queue_controller(initIslandApp());
        
        // Perform application initialization:
        auto topLevelWindow = InitInstance(hInstance, nCmdShow);
        HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIPBOARDMANAGER));

        MSG msg;
        // Main message loop:
        while (GetMessage(&msg, nullptr, 0, 0))
        {
            if (::ContentPreTranslateMessage(&msg)
                || TranslateAccelerator(msg.hwnd, hAccelTable, &msg)
                || processMessageForTabNav(topLevelWindow, msg))
            {
                continue;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    catch (const winrt::hresult_error& error)
    {
        return error.code().value;
    }
    catch (...)
    {
        return -1;
    }
    std::wstring end = L"";
    std::wcin >> end;

    return 0;
}

#pragma region Win32Desktop
//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex{};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLIPBOARDMANAGER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CLIPBOARDMANAGER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   // Initialize global strings
   LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
   LoadStringW(hInstance, IDC_CLIPBOARDMANAGER, szWindowClass, MAX_LOADSTRING);
   MyRegisterClass(hInstance);

   HWND hWnd = CreateWindowExW(0, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, InitialWindowWidth, InitialWindowHeight, nullptr, nullptr, hInstance, nullptr);
   winrt::check_pointer(hWnd);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return hWnd;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    clipmgr::utils::WindowInfo* windowInfo = clipmgr::utils::getWindowInfo(hWnd);

    switch (message)
    {
        case WM_CREATE:
        {
            createWinUIWindow(windowInfo, hWnd);
            break;
        }

        case WM_SIZE:
        {
            const int width = LOWORD(lParam);
            const int height = HIWORD(lParam);
            if (windowInfo->desktopWinXamlSrc)
            {
                windowInfo->desktopWinXamlSrc.SiteBridge().MoveAndResize(
                    {
                        0,
                        0,
                        width,
                        height
                    });
            }

            break;
        }

        case WM_ACTIVATE:
        {
            assert(windowInfo != nullptr);

            handleWindowActivation(windowInfo, LOWORD(wParam) == WA_INACTIVE);
            break;
        }

        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            
            break;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            RECT clientRect;
            GetClientRect(hWnd, &clientRect);
            HRGN bgRgn = CreateRectRgnIndirect(&clientRect);
            HBRUSH hBrush = CreateSolidBrush(RGB(31, 31, 31));
            FillRgn(hdc, bgRgn, hBrush);

            DeleteObject(bgRgn);
            DeleteObject(hBrush);

            EndPaint(hWnd, &ps);
            
            break;
        }

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }

        case WM_NCDESTROY:
        {
            auto&& mainPage = windowInfo->desktopWinXamlSrc.Content().try_as<winrt::ClipboardManager::MainPage>();
            if (mainPage)
            {
                mainPage.AppClosing();
            }

            RECT rect{};
            GetWindowRect(hWnd, &rect);
            clipmgr::Settings settings{};
            settings.insert(L"WindowPosY", static_cast<int32_t>(rect.top));
            settings.insert(L"WindowPosX", static_cast<int32_t>(rect.left));

            if (windowInfo->desktopWinXamlSrc != nullptr && windowInfo->takeFocusRequestedToken.value != 0)
            {
                windowInfo->desktopWinXamlSrc.TakeFocusRequested(windowInfo->takeFocusRequestedToken);
                windowInfo->takeFocusRequestedToken = {};
            }

            delete windowInfo;

            SetWindowLong(hWnd, GWLP_USERDATA, NULL);

            break;
        }

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
#pragma endregion

winrt::DispatcherQueueController initIslandApp()
{
    winrt::init_apartment(winrt::apartment_type::single_threaded);
    auto dispatcherQueueController{ winrt::DispatcherQueueController::CreateOnCurrentThread() };
    auto islandApp{ winrt::make<winrt::ClipboardManager::implementation::App>() };
    return dispatcherQueueController;
}

bool processMessageForTabNav(const HWND& window, MSG& msg)
{
    //TODO: If I need to handle tab to do specific stuff in my app, i should add a way to hook this function to capture the tab key before WASDK handles it.
    if (msg.message == WM_KEYDOWN && msg.wParam == VK_TAB)
    {
        const HWND focusedWindow = ::GetFocus();
        if (::GetAncestor(focusedWindow, GA_ROOT) != window)
        {
            return false;
        }

        const bool isShiftKeyDown = (GetKeyState(VK_SHIFT) & (1 << 15)) != 0;
        const HWND nextFocusedWindow = ::GetNextDlgTabItem(window, focusedWindow, isShiftKeyDown);

        clipmgr::utils::WindowInfo* windowInfo = clipmgr::utils::getWindowInfo(window);
        const HWND dwxsWindow = winrt::GetWindowFromWindowId(windowInfo->desktopWinXamlSrc.SiteBridge().WindowId());

        if (dwxsWindow == nextFocusedWindow)
        {
            winrt::XamlSourceFocusNavigationRequest request
            {
                isShiftKeyDown ? winrt::XamlSourceFocusNavigationReason::Last : winrt::XamlSourceFocusNavigationReason::First
            };

            windowInfo->desktopWinXamlSrc.NavigateFocus(request);
            return true;
        }
        else
        {
            return (IsDialogMessageW(window, &msg) == TRUE);
        }
    }

    return false;
}

void createWinUIWindow(clipmgr::utils::WindowInfo* windowInfo, const HWND& windowHandle)
{
    assert(windowInfo == nullptr);

    windowInfo = new clipmgr::utils::WindowInfo();
    SetWindowLongPtrW(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(windowInfo));

    windowInfo->desktopWinXamlSrc = winrt::DesktopWindowXamlSource();
    windowInfo->desktopWinXamlSrc.Initialize(winrt::GetWindowIdFromWindow(windowHandle));
    // Enable the DesktopWindowXamlSource to be a tab stop.
    SetWindowLong(winrt::GetWindowFromWindowId(windowInfo->desktopWinXamlSrc.SiteBridge().WindowId()), GWL_STYLE, WS_TABSTOP | WS_CHILD | WS_VISIBLE);

    // Put a new instance of our Xaml "MainPage" into our island.  This is our UI content.
    windowInfo->desktopWinXamlSrc.Content(winrt::make<winrt::ClipboardManager::implementation::MainPage>());

    clipmgr::Settings settings{};
    auto appWindow = clipmgr::utils::getCurrentAppWindow(windowHandle);
    int32_t x = settings.get<int32_t>(L"WindowPosX").value_or(10);
    int32_t y = settings.get<int32_t>(L"WindowPosY").value_or(10);
    appWindow.Move({ x, y });

    if (appWindow.TitleBar().IsCustomizationSupported())
    {
        auto&& titleBar = appWindow.TitleBar();
        titleBar.ExtendsContentIntoTitleBar(true);
        titleBar.IconShowOptions(winrt::IconShowOptions::HideIconAndSystemMenu);

        auto&& presenter = appWindow.Presenter().as<winrt::OverlappedPresenter>();
        presenter.IsMaximizable(false);
        //presenter.IsMinimizable(false);

        //titleBar.PreferredHeightOption(winrt::TitleBarHeightOption::Collapsed);
        auto&& resources = winrt::Application::Current().Resources();
        
        appWindow.TitleBar().ButtonBackgroundColor(winrt::Colors::Transparent());
        appWindow.TitleBar().ButtonInactiveBackgroundColor(winrt::Colors::Transparent());
        
        appWindow.TitleBar().ButtonInactiveForegroundColor(
            resources.TryLookup(winrt::box_value(L"AppTitleBarHoverColor")).as<winrt::Windows::UI::Color>());

        appWindow.TitleBar().ButtonHoverBackgroundColor(
            resources.TryLookup(winrt::box_value(L"ButtonHoverBackgroundColor")).as<winrt::Windows::UI::Color>());

        appWindow.TitleBar().ButtonPressedBackgroundColor(winrt::Colors::Transparent());

        appWindow.TitleBar().ButtonForegroundColor(
            resources.TryLookup(winrt::box_value(L"ButtonForegroundColor")).as<winrt::Windows::UI::Color>()
        );
        appWindow.TitleBar().ButtonHoverForegroundColor(
            resources.TryLookup(winrt::box_value(L"ButtonForegroundColor")).as<winrt::Windows::UI::Color>()
        );
        appWindow.TitleBar().ButtonPressedForegroundColor(
            resources.TryLookup(winrt::box_value(L"ButtonForegroundColor")).as<winrt::Windows::UI::Color>()
        );
    }
}

void handleWindowActivation(clipmgr::utils::WindowInfo * windowInfo, const bool & inactive)
{
    if (inactive)
    {
        windowInfo->lastFocusedWindow = GetFocus();
    }
    else if (windowInfo->lastFocusedWindow != nullptr)
    {
        SetFocus(windowInfo->lastFocusedWindow);
    }
}

wchar_t* getWindowClass()
{
    return szWindowClass;
}
