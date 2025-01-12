# Clipboard Manager
C++ WinRT - Xaml Islands - Windows App SDK.

## What is it ?
The goal of this application is to create formatted urls/links from the content copied to the clipboard. To select what content will be formatted to a specific string, you create triggers using regular expressions. If the trigger matches with the content that was just copied a notification will be sent (regular windows toast notification) with buttons if possible, just click the button you want.

If too many triggers match, you will have to open the application to choose which action to activate.

## Developement
This application has been developped using Microsoft's own librairies and SDKs. It is not cross-platform.
- [Windows App SDK](https://learn.microsoft.com/en-us/windows/apps/windows-app-sdk/).
- [XAML Islands](https://learn.microsoft.com/en-us/windows/apps/desktop/modernize/xaml-islands/xaml-islands).
- [Windows Implementation Library](https://github.com/microsoft/wil).

It also uses [boost](https://www.boost.org/).

## Installation
### Users
- Get the latest release of this application on GitHub, then download and run `ClipboardManagerV2 Installer.exe`. It should install the required packages and components needed for the application to work.

### Dev
The application has two dependencies:
- Microsoft Visual C++ Redistributable (latest) available here: *Required for the application to start*
  - [x64](https://aka.ms/vs/17/release/vc_redist.x64.exe)
  - [x86](https://aka.ms/vs/17/release/vc_redist.x86.exe)
  - [arm64](https://aka.ms/vs/17/release/vc_redist.arm64.exe)
- Windows App Runtime available here: *Note that the application will prompt you to install the latest version of the runtime on startup if it isn't installed.*
  - [Direct link](https://aka.ms/windowsappsdk/1.6/latest/windowsappruntimeinstall-x64.exe)
  - [Page link](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170) then click on the big blue button.
