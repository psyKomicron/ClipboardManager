#include "pch.h"
#include "MainPage.h"
#if __has_include("MainPage.g.cpp")
#include "MainPage.g.cpp"
#endif

#include "src/Settings.hpp"
#include "src/utils/helpers.hpp"

#include "ClipboardActionView.h"

#include <winrt/Windows.ApplicationModel.DataTransfer.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.Storage.Pickers.h>

#include <boost/property_tree/xml_parser.hpp>

#include <Shobjidl.h>

#include <iostream>

namespace impl = winrt::ClipboardManager::implementation;

namespace winrt
{
    using namespace winrt::Windows::ApplicationModel::DataTransfer;
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::ApplicationModel;
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Windows::Storage::Pickers;
}

impl::MainPage::MainPage()
{
}

winrt::Windows::Foundation::Collections::IObservableVector<winrt::ClipboardManager::ClipboardActionView> winrt::ClipboardManager::implementation::MainPage::Actions()
{
    return clipboardActionViews;
}

void winrt::ClipboardManager::implementation::MainPage::Actions(const winrt::Windows::Foundation::Collections::IObservableVector<winrt::ClipboardManager::ClipboardActionView>& value)
{
    clipboardActionViews = value;
}

winrt::Windows::Foundation::IAsyncAction impl::MainPage::Page_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const&, winrt::Windows::Foundation::IInspectable const&)
{
    auto&& clipboardHistory = co_await winrt::Clipboard::GetHistoryItemsAsync();
    for (auto&& item : clipboardHistory.Items())
    {
        auto&& itemText = co_await item.Content().GetTextAsync();
        ClipboardHistoryListView().Items().Append(box_value(itemText));
    }

    clipboardContentChangedrevoker = winrt::Clipboard::ContentChanged(winrt::auto_revoke, { this, &MainPage::ClipboardContent_Changed });

    //TODO: Get user file path from settings and ask user to create user file if it doesn't exist (in known locations).
    clipmgr::Settings settings{};
    auto userFilePath = settings.get<std::wstring>(L"UserFilePath");
    if (userFilePath.has_value())
    {
        actions = clipmgr::ClipboardAction::loadClipboardActions(userFilePath.value());

        // Print actions to debug console.
        std::wstring actionMessage = L"[MainPage]   ";
        size_t headerSize = actionMessage.size();
        actionMessage += L"Available actions: ";
        for (auto&& action : actions)
        {
            actionMessage += L"\n" + std::wstring(headerSize, L' ') + L"- " + action.label() + L": " + action.regex().str();
        }
        std::wcout << actionMessage << std::endl;
    }
    else
    {
        // Show info bar to create or locate user file.
        visualStateManager.goToState(NoClipboardActionsState);
    }
}

winrt::async impl::MainPage::LocateUserFileButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    winrt::FileOpenPicker picker{};
    picker.as<IInitializeWithWindow>()->Initialize(GetActiveWindow());
    picker.FileTypeFilter().Append(L".xml");
    
    auto&& storageFile = co_await picker.PickSingleFileAsync();
    if (storageFile)
    {
        visualStateManager.goToState(NormalActionsState);

        std::filesystem::path userFilePath{ storageFile.Path().c_str() };

        clipmgr::Settings settings{};
        settings.insert(L"UserFilePath", userFilePath.wstring());
    }
}

winrt::async impl::MainPage::CreateUserFileButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    winrt::FileSavePicker picker{};
    picker.as<IInitializeWithWindow>()->Initialize(GetActiveWindow());
    picker.SuggestedStartLocation(winrt::PickerLocationId::ComputerFolder);
    picker.FileTypeChoices().Insert(
        L"XML File", single_threaded_vector<winrt::hstring>({ L".xml" })
    );
    picker.SuggestedFileName(L"user_file.xml");

    auto&& storageFile = co_await picker.PickSaveFileAsync();
    if (storageFile)
    {
        std::filesystem::path userFilePath{ storageFile.Path().c_str() };
        clipmgr::ClipboardAction::initializeSaveFile(userFilePath);

        visualStateManager.goToState(OpenSaveFileState);
    }
}


winrt::async impl::MainPage::ClipboardContent_Changed(const winrt::IInspectable&, const winrt::IInspectable&)
{
    auto clipboardContent = winrt::Clipboard::GetContent();

    if (clipboardContent.Contains(winrt::StandardDataFormats::Text()))
    {
        auto&& text = co_await clipboardContent.GetTextAsync();

        if (clipboardActionViews.Size() == 0
            || text != clipboardActionViews.GetAt(0).Text())
        {
            auto actionView = winrt::make<::impl::ClipboardActionView>(text);

            bool hasMatch = false;
            for (auto&& action : actions)
            {
                if (action.match(std::wstring(text)))
                {
                    actionView.AddAction(action.format(), action.label(), L"");
                    hasMatch = true;
                }
            }

            if (hasMatch)
            {
                clipboardActionViews.InsertAt(0, actionView);
                actionView.Removed([this](auto&& sender, auto&&)
                {
                    for (uint32_t i = 0; i < clipboardActionViews.Size(); i++)
                    {
                        auto&& action = clipboardActionViews.GetAt(i);
                        if (sender == action)
                        {
                            clipboardActionViews.RemoveAt(i);
                        }
                    }
                });
            }
            else
            {
                std::wcout << L"[MainPage]   No matching actions found for '" << std::wstring(text) << L"'. Actions " << (actions.empty() ? L"not loaded" : L"loaded") << std::endl;
            }
        }
        else
        {
            std::wcout << L"[MainPage]   Clipboard action is already added (" << std::wstring(text) << L")." << std::endl;
        }
    }
}

void impl::MainPage::App_Closing(const winrt::Windows::Foundation::IInspectable&, const winrt::Windows::Foundation::IInspectable&)
{
    clipmgr::Settings settings{};
    auto userFileOptional = settings.get<std::wstring>(L"UserFilePath");

    /*if (userFileOptional.has_value())
    {
        
    }*/

    std::filesystem::path path{ userFileOptional.value_or(L"c:\\users\\julie\\documents\\clipboardmanager\\user_file.xml") };

    boost::property_tree::wptree tree{};
    boost::property_tree::read_xml(path.string(), tree);

    tree.erase(L"settings.history");

    for (auto&& view : clipboardActionViews)
    {
        tree.add(L"history.item", std::wstring(view.Text()));
    }

    boost::property_tree::write_xml(path.string(), tree);
}
