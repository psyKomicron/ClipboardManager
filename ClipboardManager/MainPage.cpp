#include "pch.h"
#include "MainPage.h"
#if __has_include("MainPage.g.cpp")
#include "MainPage.g.cpp"
#endif

#include "src/Settings.hpp"
#include "src/utils/helpers.hpp"
#include "src/notifs/ToastNotification.hpp"
#include "src/notifs/win_toasts.hpp"

#include "ClipboardActionView.h"
#include "ClipboardActionEditor.h"

#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.ApplicationModel.DataTransfer.h>
#include <winrt/Windows.ApplicationModel.Resources.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Microsoft.Windows.AppNotifications.h>
#include <winrt/Microsoft.Windows.AppNotifications.Builder.h>

#include <boost/property_tree/xml_parser.hpp>

#include <Shobjidl.h>

#include <iostream>

namespace impl = winrt::ClipboardManager::implementation;
namespace winrt
{
    using namespace winrt::Windows::ApplicationModel::DataTransfer;
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::ApplicationModel;
    using namespace winrt::Windows::ApplicationModel::Resources;
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Windows::Storage::Pickers;
    using namespace winrt::Microsoft::Windows::AppNotifications;
    using namespace winrt::Microsoft::Windows::AppNotifications::Builder;
}

impl::MainPage::MainPage()
{
    auto userFilePath = localSettings.get<std::wstring>(L"UserFilePath");
    if (userFilePath.has_value() && clipmgr::utils::pathExists(userFilePath.value()))
    {
        actions = clipmgr::ClipboardAction::loadClipboardActions(userFilePath.value());

        // Print actions to debug console.
        std::wstring actionMessage = L"\nAvailable actions: ";
        for (auto&& action : actions)
        {
            actionMessage += L"\n\t- " + action.label() + L": " + action.regex().str();
        }
        logger.info(actionMessage);
    }

    if (actions.empty())
    {
        logger.info(L"'UserFilePath' doesn't exist or the path of 'UserFilePath' doesn't exist.");
        // Show info bar to create or locate user file.
        visualStateManager.goToState(NoClipboardActionsState);
    }
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
}

winrt::async impl::MainPage::LocateUserFileButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    winrt::FileOpenPicker picker{};
    picker.as<IInitializeWithWindow>()->Initialize(GetActiveWindow());
    picker.FileTypeFilter().Append(L".xml");
    
    auto&& storageFile = co_await picker.PickSingleFileAsync();
    if (storageFile)
    {
        std::filesystem::path userFilePath{ storageFile.Path().c_str() };

        clipmgr::Settings settings{};
        settings.insert(L"UserFilePath", userFilePath.wstring());

        visualStateManager.goToState(ViewActionsState);
    }
}

winrt::async impl::MainPage::CreateUserFileButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    winrt::FileSavePicker picker{};
    picker.as<IInitializeWithWindow>()->Initialize(GetActiveWindow());
    picker.SuggestedStartLocation(winrt::PickerLocationId::Unspecified);
    picker.FileTypeChoices().Insert(
        L"XML Files", single_threaded_vector<winrt::hstring>({ L".xml" })
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

void impl::MainPage::ViewActionsButton_Click(winrt::IInspectable const&, winrt::RoutedEventArgs const&)
{
    Pivot().SelectedIndex(2);
}

void impl::MainPage::ClipboadActionsListPivot_Loading(winrt::IInspectable const&, winrt::IInspectable const&)
{
    if (actions.empty())
    {
        visualStateManager.goToState(NoClipboardActionsToDisplayState);
        logger.debug(L"ClipboadActionsListPivot_Loading > No clipboard actions to load.");
    }
    else
    {
        logger.debug(std::format(L"ClipboadActionsListPivot_Loading > Loading {} clipboard actions.", actions.size()));
        
        for (auto&& action : actions)
        {
            auto viewModel = make<ClipboardManager::implementation::ClipboardActionEditor>();
            viewModel.ActionFormat(action.format());
            viewModel.ActionLabel(action.label());
            viewModel.ActionRegex(action.regex().str());
            viewModel.ActionEnabled(action.enabled());
            //auto flags = static_cast<uint32_t>(action.regex().flags());

            ClipboardActionsListView().Items().Append(viewModel);
        }
    }
}


winrt::async impl::MainPage::ClipboardContent_Changed(const winrt::IInspectable&, const winrt::IInspectable&)
{
    auto clipboardContent = winrt::Clipboard::GetContent();
    auto&& text = std::wstring(co_await clipboardContent.GetTextAsync());
    if (!clipboardContent.Contains(winrt::StandardDataFormats::Text()))
    {
        logger.info(L"Clipboard action is already added (" + text + L").");
        co_return;
    }

    bool addExisting = localSettings.get<bool>(L"AddDuplicatedActions").value_or(false);
    if (addExisting 
        || clipboardActionViews.Size() == 0
        || text != clipboardActionViews.GetAt(0).Text())
    {
        auto actionView = winrt::make<::impl::ClipboardActionView>(text.c_str());

        clipmgr::ToastNotification notif{};
        std::vector<std::pair<std::wstring, std::wstring>> buttons{};

        bool hasMatch = false;
        for (auto&& action : actions)
        {
            if (action.enabled() && action.match(text))
            {
                hasMatch = true;

                actionView.AddAction(action.format(), action.label(), L"");

                auto url = std::vformat(action.format(), std::make_wformat_args(text));
                buttons.push_back({ action.label(), std::format(L"open&url={}", url) });
            }
        }

        if (!hasMatch)
        {
            logger.info(L"No matching actions found for '" + std::wstring(text) + L"'. Actions " + (actions.empty() ? L"not loaded" : L"loaded"));
            co_return;
        }

        clipboardActionViews.InsertAt(0, actionView);
        actionView.Removed([this](auto&& sender, auto&&)
        {
            for (uint32_t i = 0; i < clipboardActionViews.Size(); i++)
            {
                if (sender == clipboardActionViews.GetAt(i))
                {
                    clipboardActionViews.RemoveAt(i);
                }
            }
        });

        try
        {
            winrt::ResourceLoader resLoader{};
            if (!notif.tryAddButtons(buttons))
            {
                notif.addText(std::wstring(resLoader.GetString(L"ToastNotification_TooManyButtons")));
                notif.addButton(std::wstring(resLoader.GetString(L"ToastNotification_OpenApp")), L"action=focus");
            }
            else
            {
                notif.addText(std::wstring(resLoader.GetString(L"ToastNotification_ClickAction")));
            }

            notif.send();
            logger.debug(L"Sent notification");
        }
        catch (winrt::hresult_error err)
        {
            logger.error(L"ToastNotification::send() failed : " + std::wstring(err.message()));
        }
    }
}

void impl::MainPage::App_Closing(const winrt::Windows::Foundation::IInspectable&, const winrt::Windows::Foundation::IInspectable&)
{
    auto userFilePath = localSettings.get<std::filesystem::path>(L"UserFilePath");

    if (userFilePath.has_value())
    {
        boost::property_tree::wptree tree{};
        boost::property_tree::read_xml(userFilePath.value().string(), tree);

        tree.erase(L"settings.history");

        for (auto&& view : clipboardActionViews)
        {
            tree.add(L"history.item", std::wstring(view.Text()));
        }

        boost::property_tree::write_xml(userFilePath.value().string(), tree);
    }
    else
    {
        logger.debug(L"User file path not saved in settings, impossible to save history.");
    }
}
