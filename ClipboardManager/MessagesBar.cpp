#include "pch.h"
#include "MessagesBar.h"
#if __has_include("MessagesBar.g.cpp")
#include "MessagesBar.g.cpp"
#endif

namespace xaml
{
    using namespace winrt::Microsoft::UI::Xaml::Controls;
}

namespace winrt::ClipboardManager::implementation
{
    void MessagesBar::Add(const winrt::hstring& title, const winrt::hstring& message, const xaml::InfoBarSeverity& severity)
    {
        infoBars.push_back(CreateInfoBar(title, message, severity));

        InfoBadge().Value(static_cast<int32_t>(infoBars.size()));
        PipsPager().NumberOfPages(static_cast<int32_t>(infoBars.size()));

        if (infoBars.size() == 1)
        {
            LoadFirst();
        }
    }

    void MessagesBar::Add(const winrt::hstring& titleKey, const winrt::hstring& altTitle, const winrt::hstring& messageKey, const winrt::hstring& messageAlt)
    {
        auto title = resLoader.getOrAlt(titleKey, altTitle);
        auto message = resLoader.getOrAlt(messageKey, messageAlt);

        Add(title, message, xaml::InfoBarSeverity::Informational);
    }

    void MessagesBar::AddMessage(const winrt::hstring& message)
    {
        Add({}, message, xaml::InfoBarSeverity::Informational);
    }

    void MessagesBar::AddMessage(const winrt::hstring& messageKey, const winrt::hstring& messageAlt)
    {
        AddMessage(resLoader.getOrAlt(messageKey, messageAlt));
    }

    void MessagesBar::AddWarning(const winrt::hstring& messageKey, const winrt::hstring& messageAlt)
    {
        Add({}, resLoader.getOrAlt(messageKey, messageAlt), xaml::InfoBarSeverity::Warning);
    }

    void MessagesBar::AddError(const winrt::hstring& messageKey, const winrt::hstring& messageAlt)
    {
        Add({}, resLoader.getOrAlt(messageKey, messageAlt), xaml::InfoBarSeverity::Error);
    }

    void MessagesBar::AddError(const winrt::hstring& titleKey, const winrt::hstring& altTitle, const winrt::hstring& messageKey, const winrt::hstring& messageAlt)
    {
        Add(resLoader.getOrAlt(titleKey, altTitle), resLoader.getOrAlt(messageKey, messageAlt), xaml::InfoBarSeverity::Error);
    }

    void MessagesBar::AddContent(const winrt::hstring& titleContent, const winrt::hstring& messageContent, const winrt::Windows::Foundation::IInspectable& content, const winrt::Microsoft::UI::Xaml::Controls::InfoBarSeverity& severity)
    {
        auto&& infoBar = CreateInfoBar(titleContent, messageContent, severity);
        infoBar.Content(content);
        infoBar.Severity(severity);

        infoBars.push_back(std::move(infoBar));

        InfoBadge().Value(static_cast<int32_t>(infoBars.size()));
        PipsPager().NumberOfPages(static_cast<int32_t>(infoBars.size()));

        if (infoBars.size() == 1)
        {
            LoadFirst();
        }
    }


    void MessagesBar::UserControl_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        // Load first control
        loaded = true;
        LoadFirst();
    }

    void MessagesBar::LeftButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        LoadPrevious();
    }

    void MessagesBar::RightButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        LoadNext();
    }

    void MessagesBar::PipsPager_SelectedIndexChanged(winrt::Microsoft::UI::Xaml::Controls::PipsPager const& sender, winrt::Microsoft::UI::Xaml::Controls::PipsPagerSelectedIndexChangedEventArgs const& args)
    {
        auto selectedIndex = static_cast<size_t>(sender.SelectedPageIndex());

        if (selectedIndex < infoBars.size() && selectedIndex >= 0)
        {
            index = selectedIndex;
            ContentPresenter().Content(infoBars[index]);
            //PipsPager().SelectedPageIndex(index);
        }
    }


    void MessagesBar::LoadFirst()
    {
        if (loaded && !infoBars.empty())
        {
            ContentPresenter().Content(infoBars[0]);
            PipsPager().NumberOfPages(static_cast<int32_t>(infoBars.size()));
            visualStateManager.goToState(openState);
        }
    }

    void MessagesBar::LoadNext()
    {
        if ((index + 1) < infoBars.size())
        {
            ContentPresenter().Content(infoBars[++index]);
            PipsPager().SelectedPageIndex(static_cast<int32_t>(index));
        }
    }

    void MessagesBar::LoadPrevious()
    {
        if (index > 0)
        {
            ContentPresenter().Content(infoBars[--index]);
            PipsPager().SelectedPageIndex(static_cast<int32_t>(index));
        }
    }

    Microsoft::UI::Xaml::Controls::InfoBar MessagesBar::CreateInfoBar(
        const winrt::hstring& title, const winrt::hstring& message, const winrt::Microsoft::UI::Xaml::Controls::InfoBarSeverity& severity, const Windows::Foundation::IInspectable& content)
    {
        Microsoft::UI::Xaml::Controls::InfoBar infoBar{};

        if (!title.empty())
        {
            infoBar.Title(title);
        }

        if (!message.empty())
        {
            infoBar.Message(message);
        }

        if (content != nullptr)
        {
            infoBar.Content(content);
        }

        infoBar.Severity(severity);
        infoBar.IsOpen(true);

        infoBar.Closed([this](auto&& infoBar, auto&&)
        {
            size_t i = 0;
            for (; i < infoBars.size(); i++)
            {
                if (infoBars[i] == infoBar)
                {
                    infoBars.erase(infoBars.begin() + i);
                    break;
                }
            }

            PipsPager().NumberOfPages(static_cast<int32_t>(infoBars.size()));
            InfoBadge().Value(static_cast<int32_t>(infoBars.size()));

            if (i < infoBars.size())
            {
                ContentPresenter().Content(infoBars[index]);
            }

            if (infoBars.empty())
            {
                visualStateManager.goToState(closedState);
            }
        });

        return infoBar;
    }
}
