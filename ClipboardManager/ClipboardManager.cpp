// ClipboardManager.cpp : Defines the entry point for the application.
#include "pch.h"
#include "ClipboardManager.h"

#include "lib/utils/helpers.hpp"
#include "lib/utils/Console.hpp"
#include "lib/Settings.hpp"

#include "App.xaml.h"
#include "MainPage.h"

#include <Microsoft.UI.Dispatching.Interop.h> // For ContentPreTranslateMessage
#include <winrt/Microsoft.Graphics.Display.h>
#include <winrt/Windows.Graphics.h>

#include <memory>
#include <iostream>
#include <chrono>
#include <map>

namespace winrt
{
    using namespace winrt::Microsoft::UI;
    using namespace winrt::Microsoft::UI::Dispatching;
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Hosting;
    using namespace winrt::Microsoft::UI::Xaml::Markup;
    using namespace winrt::Microsoft::UI::Windowing;
    using namespace winrt::Microsoft::Graphics::Display;
    using namespace winrt::Windows::Graphics;
}

#pragma region Header
#define MAX_LOADSTRING 100

// Global Variables:
WCHAR szTitle[MAX_LOADSTRING];// The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];// the main window class name

constexpr uint32_t InitialWindowWidth = 470;
constexpr uint32_t InitialWindowHeight = 700;

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance);
HWND InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

winrt::DispatcherQueueController initIslandApp();
bool processMessageForTabNav(const HWND& window, MSG& msg);
void createWinUIWindow(clip::utils::WindowInfo* windowInfo, const HWND& windowHandle);
void handleWindowActivation(clip::utils::WindowInfo* windowInfo, const bool& inactive);
#pragma endregion

#ifdef _DEBUG
#define ENABLE_CONSOLE
#endif


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int nCmdShow)
{
#ifdef ENABLE_CONSOLE
    clip::utils::Console console{};
#endif // ENABLE_CONSOLE

    clip::utils::Logger logger{ L"WinMain" };

    try
    {
        auto dispatcherQueueController = clip::utils::managed_dispatcher_queue_controller(initIslandApp());

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
        logger.error(std::to_wstring(error.code().value));
        return error.code().value;
    }
    catch (...)
    {
        logger.error(L"Unknown error occured.");
        //std::wcout << L"Unknown error occured." << std::endl;
    }

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

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLIPBOARDMANAGER));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CLIPBOARDMANAGER);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
    static clip::utils::Logger logger{ L"WndProc" };

    clip::utils::WindowInfo* windowInfo = clip::utils::getWindowInfo(hWnd);
    std::optional<winrt::ClipboardManager::MainPage> mainPage{};
    if (windowInfo != nullptr && windowInfo->desktopWinXamlSrc != nullptr)
    {
        auto&& content = windowInfo->desktopWinXamlSrc.Content();
        if (content != nullptr)
        {
            mainPage = content.try_as<winrt::ClipboardManager::MainPage>();
        }
    }

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
            if (windowInfo && windowInfo->desktopWinXamlSrc)
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

            if (mainPage)
            {
                mainPage.value().ReceiveWindowMessage(message, wParam);
            }
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
            if (mainPage)
            {
                mainPage.value().AppClosing();
            }

            RECT rect{};
            GetWindowRect(hWnd, &rect);
            clip::Settings settings{};
            settings.insert(L"WindowPosY", static_cast<int32_t>(rect.top));
            settings.insert(L"WindowPosX", static_cast<int32_t>(rect.left));
            settings.insert(L"WindowWidth", static_cast<int32_t>(rect.right - rect.left));
            settings.insert(L"WindowHeight", static_cast<int32_t>(rect.bottom - rect.top));

            if (windowInfo && windowInfo->desktopWinXamlSrc != nullptr && windowInfo->takeFocusRequestedToken.value != 0)
            {
                windowInfo->desktopWinXamlSrc.TakeFocusRequested(windowInfo->takeFocusRequestedToken);
                windowInfo->takeFocusRequestedToken = {};
            }

            delete windowInfo;

            SetWindowLong(hWnd, GWLP_USERDATA, NULL);

            RemoveClipboardFormatListener(hWnd);

            break;
        }

        case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
            lpMMI->ptMinTrackSize.x = 400;
            lpMMI->ptMinTrackSize.y = 300;
            break;
        }

        case WM_CLIPBOARDUPDATE:
            if (mainPage)
            {
                mainPage.value().ReceiveWindowMessage(message, wParam);
            }
            break;
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
    if (msg.message == WM_KEYDOWN && msg.wParam == VK_TAB)
    {
        const HWND focusedWindow = ::GetFocus();
        if (::GetAncestor(focusedWindow, GA_ROOT) != window)
        {
            return false;
        }

        const bool isShiftKeyDown = (GetKeyState(VK_SHIFT) & (1 << 15)) != 0;
        const HWND nextFocusedWindow = ::GetNextDlgTabItem(window, focusedWindow, isShiftKeyDown);

        clip::utils::WindowInfo* windowInfo = clip::utils::getWindowInfo(window);
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

void createWinUIWindow(clip::utils::WindowInfo* windowInfo, const HWND& windowHandle)
{
    clip::utils::Logger logger{ L"createWinUIWindow" };

    assert(windowInfo == nullptr);

    windowInfo = new clip::utils::WindowInfo();
    SetWindowLongPtrW(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(windowInfo));

    windowInfo->desktopWinXamlSrc = winrt::DesktopWindowXamlSource();
    windowInfo->desktopWinXamlSrc.Initialize(winrt::GetWindowIdFromWindow(windowHandle));
    logger.info(L"Initialized `DesktopWindowXamlSource`.");
    // Enable the DesktopWindowXamlSource to be a tab stop.
    SetWindowLong(winrt::GetWindowFromWindowId(windowInfo->desktopWinXamlSrc.SiteBridge().WindowId()), GWL_STYLE, WS_TABSTOP | WS_CHILD | WS_VISIBLE);

    auto appWindow = clip::utils::getCurrentAppWindow(windowHandle);

    // Put a new instance of our Xaml "MainPage" into our island.  This is our UI content.
    windowInfo->desktopWinXamlSrc.Content(winrt::make<winrt::ClipboardManager::implementation::MainPage>(appWindow));
    logger.info(L"Window content created. Moving app window.");

    clip::Settings settings{};
    int32_t x = settings.get<int32_t>(L"WindowPosX").value_or(10);
    int32_t y = settings.get<int32_t>(L"WindowPosY").value_or(10);
    int32_t width = settings.get<int32_t>(L"WindowWidth").value_or(InitialWindowWidth);
    int32_t height = settings.get<int32_t>(L"WindowHeight").value_or(InitialWindowHeight);

    auto display = winrt::DisplayArea::GetFromPoint(winrt::PointInt32(x, y), winrt::DisplayAreaFallback::None);
    if (display == nullptr)
    {
        display = winrt::DisplayArea::GetFromPoint(winrt::PointInt32(x, y), winrt::DisplayAreaFallback::Primary);
        x = (display.WorkArea().Width / 2) - (width / 2);
        y = (display.WorkArea().Height / 2) - (height / 2);
    }

    appWindow.MoveAndResize({ x, y, width, height });

    if (appWindow.TitleBar().IsCustomizationSupported())
    {
        auto&& titleBar = appWindow.TitleBar();
        titleBar.ExtendsContentIntoTitleBar(true);
        titleBar.IconShowOptions(winrt::IconShowOptions::HideIconAndSystemMenu);

        auto&& presenter = appWindow.Presenter().try_as<winrt::OverlappedPresenter>();
        if (presenter != nullptr)
        {
            presenter.IsMinimizable(
                settings.get<bool>(L"AllowWindowMinimize").value_or(true));
            presenter.IsMaximizable(
                settings.get<bool>(L"AllowWindowMaximize").value_or(false));
        }

        auto&& resources = winrt::Application::Current().Resources();

        appWindow.TitleBar().ButtonBackgroundColor(winrt::Colors::Transparent());
        appWindow.TitleBar().ButtonInactiveBackgroundColor(winrt::Colors::Transparent());

        appWindow.TitleBar().ButtonInactiveForegroundColor(
            resources.TryLookup(winrt::box_value(L"AppTitleBarHoverColor")).as<winrt::Windows::UI::Color>());

        appWindow.TitleBar().ButtonHoverBackgroundColor(
            resources.TryLookup(winrt::box_value(L"ButtonHoverBackgroundColor")).as<winrt::Windows::UI::Color>());

        appWindow.TitleBar().ButtonPressedBackgroundColor(winrt::Colors::Transparent());

        appWindow.TitleBar().ButtonForegroundColor(
            resources.TryLookup(winrt::box_value(L"ButtonForegroundColor")).as<winrt::Windows::UI::Color>());
        appWindow.TitleBar().ButtonHoverForegroundColor(
            resources.TryLookup(winrt::box_value(L"ButtonForegroundColor")).as<winrt::Windows::UI::Color>());
        appWindow.TitleBar().ButtonPressedForegroundColor(
            resources.TryLookup(winrt::box_value(L"ButtonForegroundColor")).as<winrt::Windows::UI::Color>());
    }

    if (AddClipboardFormatListener(windowHandle)) // outlook-bug
    {
        logger.info(L"Added to clipboard format listeners.");
    }
    else
    {
        logger.error(L"Not added to clipboard format listeners.");
    }

    logger.info(L"Created WinUI window.");
}

void handleWindowActivation(clip::utils::WindowInfo* windowInfo, const bool& inactive)
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
