#include "pch.h"
#include "SearchSuggestionView.h"
#if __has_include("SearchSuggestionView.g.cpp")
#include "SearchSuggestionView.g.cpp"
#endif

namespace xaml
{
    using namespace winrt::Microsoft::UI::Xaml;
}

namespace winrt::ClipboardManager::implementation
{
    IInspectable SearchSuggestionView::Icon() const
    {
        return _icon.get();
    }

    void SearchSuggestionView::Icon(const IInspectable& value)
    {
        _icon.set(value);
    }

    IInspectable SearchSuggestionView::Suggestion() const
    {
        return _suggestion.get();
    }

    void SearchSuggestionView::Suggestion(const IInspectable& value)
    {
        _suggestion.set(value);
    }

    IInspectable SearchSuggestionView::RightContent() const
    {
        return _rightContent.get();
    }

    void SearchSuggestionView::RightContent(const IInspectable& value)
    {
        _rightContent.set(value);
    }

    hstring SearchSuggestionView::Subtitle() const
    {
        return _subtitle.get();
    }

    void SearchSuggestionView::Subtitle(const hstring& value)
    {
        _subtitle.set(value);
    }

    Windows::Foundation::IInspectable SearchSuggestionView::asInspectable()
    {
        return *this;
    }
}
