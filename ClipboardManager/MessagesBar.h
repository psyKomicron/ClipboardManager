#pragma once
#include "MessagesBar.g.h"

#include "src/ui/VisualStateManager.hpp"
#include "src/utils/ResLoader.hpp"

namespace winrt::ClipboardManager::implementation
{
    struct MessagesBar : MessagesBarT<MessagesBar>
    {
    public:
        MessagesBar() = default;

        void Add(const winrt::hstring& title, const winrt::hstring& message);
        void Add(const winrt::hstring& titleKey, const winrt::hstring& altTitle, const winrt::hstring& messageKey, const winrt::hstring& messageAlt);
        void AddMessage(const winrt::hstring& message);
        void AddMessage(const winrt::hstring& messageKey, const winrt::hstring& messageAlt);

        void UserControl_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void LeftButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RightButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void PipsPager_SelectedIndexChanged(winrt::Microsoft::UI::Xaml::Controls::PipsPager const& sender, winrt::Microsoft::UI::Xaml::Controls::PipsPagerSelectedIndexChangedEventArgs const& args);

    private:
        bool loaded = false;
        std::vector<Microsoft::UI::Xaml::Controls::InfoBar> infoBars{};
        size_t index = 0;
        clip::ui::VisualStateManager<MessagesBar> visualStateManager{ *this };
        clip::ui::VisualState<MessagesBar> openState{ L"Open", 0, true };
        clip::ui::VisualState<MessagesBar> closedState{ L"Closed", 0, false };
        clip::utils::ResLoader resLoader{};

        void LoadFirst();
        void LoadNext();
        void LoadPrevious();
        Microsoft::UI::Xaml::Controls::InfoBar CreateInfoBar(const winrt::hstring& title, const winrt::hstring& message, const Windows::Foundation::IInspectable& content = nullptr);
    };
}

namespace winrt::ClipboardManager::factory_implementation
{
    struct MessagesBar : MessagesBarT<MessagesBar, implementation::MessagesBar>
    {
    };
}