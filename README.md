# Clipboard Manager
C++ WinRT - Xaml Islands - Windows App SDK.

## What is it ?
The goal of this application is to create formatted urls/links from the content copied to the clipboard. To select what content will be formatted to a specific string, you create triggers using regular expressions. If the trigger matches with the content that was just copied a notification will be sent (regular windows toast notification) with buttons if possible, just click the button you want.
A notification can look like that:

![image](https://github.com/user-attachments/assets/66cb562a-536e-491a-9f16-2163faea5649)

If too many triggers match, you will have to open the application to choose which action to activate (ex: ATEL).

## Developement
This application has been developped using Microsoft's own librairies and SDKs. It is not cross-platform.
- [Windows App SDK](https://learn.microsoft.com/en-us/windows/apps/windows-app-sdk/).
- [XAML Islands](https://learn.microsoft.com/en-us/windows/apps/desktop/modernize/xaml-islands/xaml-islands).
- [Windows Implementation Library](https://github.com/microsoft/wil).

It also uses [boost](https://www.boost.org/).

## Installation
The application has two dependencies:
- Microsoft Visual C++ Redistributable (latest) available here: *Required for the application to start*
  - [x64](https://aka.ms/vs/17/release/vc_redist.x64.exe)
  - [x86](https://aka.ms/vs/17/release/vc_redist.x86.exe)
  - [arm64](https://aka.ms/vs/17/release/vc_redist.arm64.exe)
- Windows App Runtime available here: *Note that the application will prompt you to install the latest version of the runtime on startup if it isn't installed.*
  - [Direct link](https://aka.ms/windowsappsdk/1.6/latest/windowsappruntimeinstall-x64.exe)
  - [Page link](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170) then click on the big blue button.
 
Install MSVC Redistributables then start the application to be prompted to install the latest version of the Windows App Runtime, or install both then start the application.

The application directory should look that this once unziped:
```
\ClipboardManager\
  - App.xbf
  - ClipboardActionEditor
  - ClipboardActionView
  - ClipboardHistoryItemView
  - ClipboardManager.exe <--- This starts the application
  - ClipboardManager.exp
  - ClipboardManager.ico
  - ClipboardManager.lib
  - resources.pri
  - ClipboardManager.winmd
  - ClipboardTriggerEditControl.xbf
  - HostControl.xbf
  - MainPage.xbf
  - MessagesBar.xbf
  - Microsoft.Web.WebView2.Core.dll
  - Microsoft.Web.WebView2.Core.winmd
  - Microsoft.WindowsAppRuntime.Bootstrap.dll
  - RegexTesterControl.xbf
  - SettingsPage.xbf
```
Start `ClipboardManager.exe` as any other application.
Create a shortcut to `ClipboardManager.exe` if you want to add it to another directory.
