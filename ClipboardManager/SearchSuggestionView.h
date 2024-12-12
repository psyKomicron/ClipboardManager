#pragma once

#include "SearchSuggestionView.g.h"

#include "src/ui/ListenablePropertyValue.hpp"
#include "src/ui/VisualStateManager.hpp"

namespace winrt::ClipboardManager::implementation
{
    struct SearchSuggestionView : SearchSuggestionViewT<SearchSuggestionView>, clip::ui::PropertyChangedClass
    {
    public:
        SearchSuggestionView() = default;

        IInspectable Icon() const;
        void Icon(const IInspectable& value);
        IInspectable Suggestion() const;
        void Suggestion(const IInspectable& value);
        IInspectable RightContent() const;
        void RightContent(const IInspectable& value);
        hstring Subtitle() const;
        void Subtitle(const hstring& value);

        // Inherited via PropertyChangedClass
        Windows::Foundation::IInspectable asInspectable() override;

        void ShowCopiedToClipboard(const hstring& triggerLabel);

    private:
        clip::ui::ListenablePropertyValue<IInspectable> _icon{ BIND(&SearchSuggestionView::raisePropertyChanged), nullptr };
        clip::ui::ListenablePropertyValue<IInspectable> _suggestion{ BIND(&SearchSuggestionView::raisePropertyChanged), nullptr };
        clip::ui::ListenablePropertyValue<IInspectable> _rightContent{ BIND(&SearchSuggestionView::raisePropertyChanged), nullptr };
        clip::ui::ListenablePropertyValue<hstring> _subtitle{ BIND(&SearchSuggestionView::raisePropertyChanged), L"" };
        clip::ui::VisualStateManager<SearchSuggestionView> visualStateManager{ *this };
        clip::ui::VisualState<SearchSuggestionView> clipboardNoneState{ L"ClipboardNone", 0, true };
        clip::ui::VisualState<SearchSuggestionView> clipboardCopiedState{ L"ClipboardCopied", 0, false };

    public:
        void UserControl_RightTapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::RightTappedRoutedEventArgs const& e);
        void ClipboardStatesStoryboard_Completed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
    };
}

namespace winrt::ClipboardManager::factory_implementation
{
    struct SearchSuggestionView : SearchSuggestionViewT<SearchSuggestionView, implementation::SearchSuggestionView>
    {
    };
}
