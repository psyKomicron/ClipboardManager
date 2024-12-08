#pragma once

#include "SearchSuggestionView.g.h"

#include "src/ui/ListenablePropertyValue.hpp"

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

    private:
        clip::ui::ListenablePropertyValue<IInspectable> _icon{ BIND(&SearchSuggestionView::raisePropertyChanged), nullptr };
        clip::ui::ListenablePropertyValue<IInspectable> _suggestion{ BIND(&SearchSuggestionView::raisePropertyChanged), nullptr };
        clip::ui::ListenablePropertyValue<IInspectable> _rightContent{ BIND(&SearchSuggestionView::raisePropertyChanged), nullptr };
        clip::ui::ListenablePropertyValue<hstring> _subtitle{ BIND(&SearchSuggestionView::raisePropertyChanged), L"" };
    };
}

namespace winrt::ClipboardManager::factory_implementation
{
    struct SearchSuggestionView : SearchSuggestionViewT<SearchSuggestionView, implementation::SearchSuggestionView>
    {
    };
}
